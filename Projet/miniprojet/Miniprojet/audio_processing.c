#include <main.h>
#include <chprintf.h>
#include <audio/microphone.h>
#include <audio_processing.h>
#include <arm_math.h>
#include <arm_const_structs.h>
#include <leds_animations.h>
#include <audio/play_sound_file.h>


//semaphore
static BSEMAPHORE_DECL(sequAquired, TRUE);

#define MIN_VALUE_THRESHOLD	10000 

#define MIN_FREQ		10	//we don't analyze before this index to not use resources for nothing
#define MAX_FREQ		50	//we don't analyze after this index to not use resources for nothing

#define NO_PEAK -1

//2 times FFT_SIZE because this array contain complex numbers (real + imaginary)
static float micFront_cmplx_input[2 * FFT_SIZE];
//Array containing the computed magnitude of the complex numbers
static float micFront_output[FFT_SIZE];

static int8_t frequ;
static int8_t sequ[MAX_MOVES];
static uint8_t sequ_size = 0;

//*****FORWARD DECLARATION*****
bool is_same_freq(int8_t input_freq, int8_t match_freq);
bool sequ_ended(void);
void serial_print_sequ(void);
void record_sequ(void);
void wait_for_start_sequ(void);
bool is_adjacent(uint8_t current_position, uint8_t next_position);
void check_errors(void);
void error_mode(void);
//*****************************

static THD_WORKING_AREA(waThdGetAudioSeq, 1024);
static THD_FUNCTION(ThdGetAudioSeq, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    set_led_state(IDLE);
	wait_for_start_sequ();

	set_led_state(LISTENING);
	record_sequ();
	#ifdef DEBUG
	serial_print_sequ();
	#endif
	check_errors();
	chBSemSignal(&sequAquired);
}

/*
 * Returns 1 if the positions are adjacent, otherwise returns 0
 */
bool is_adjacent(uint8_t current_position, uint8_t next_position){
	uint8_t x0 = --current_position / 3;
	uint8_t y0 = current_position % 3;
	uint8_t x1 = --next_position / 3;
	uint8_t y1 = next_position % 3;

	uint8_t dx = abs(x0-x1);
	uint8_t dy = abs(y0-y1);

	return ((dx == 1 && dy == 0) || (dx == 0 && dy == 1));
}

/*
 *	checks in the position array if some positions are not adjacent,
 *	which will trigger the error mode
 */
void check_errors(void){
	for (uint8_t i = 0; i < sequ_size - 1; ++i){
		if (!is_adjacent(sequ[i], sequ[i+1])){
			error_mode();
		}
	}
}

/*
 * Sets the e-puck in error mode  (sound + leds)
 */
void error_mode(void){
	setSoundFileVolume(50);
	playSoundFile("error.wav",SF_SIMPLE_PLAY);
	waitSoundFileHasFinished();
	set_led_state(ERROR_MODE);
	while(1){
		; //stays in error mode until reset
	}
}

/*
 * Print the received sequence over serial
 */
void serial_print_sequ(void){
	chprintf((BaseSequentialStream *) &SD3, "\nSequ: [ ");
	for (uint8_t i = 0; i < sequ_size; ++i){
		chprintf((BaseSequentialStream *) &SD3, "%d ", sequ[i]);
	}
	chprintf((BaseSequentialStream *) &SD3, "]\n");
}

/*
 * Check if two frequences are the same, allowing a delta of 47Hz
 */
bool is_same_freq(int8_t input_freq, int8_t match_freq){
	return ((input_freq - 1) <= match_freq && (input_freq + 1) >= match_freq);
}

/*
 * Wait for the frequency to change
 */
void wait_for_next_peak(int8_t old_freq){
	while (is_same_freq(frequ, old_freq)) {
		chThdYield();
	}
}

/*
 * Wait until it receives the start sequence
 */
void wait_for_start_sequ(void){

    int8_t startSequence[] = {29, 32, 36, 29, 32, 44};
    int8_t old_freq = NO_PEAK;
	uint8_t i = 0;

	while (i < sizeof startSequence / sizeof startSequence[0]){
		wait_for_next_peak(old_freq);
		if (is_same_freq(frequ,startSequence[i])){
			old_freq = frequ;
			i++;
		} else {
			old_freq = NO_PEAK;
			i = 0;
		}
	}
}

/*
 * Check if it has received the end sequence
 */
bool sequ_ended(void){

    int8_t endSequence[] = {44, 29, 32, 29};

    if (sequ_size <= 4) return false;
    for (uint8_t i = 0; i < sizeof endSequence / sizeof endSequence[0]; ++i){
    	if (!is_same_freq(sequ[sequ_size-4+i], endSequence[i])){
    		return false;
    	}
    }
	return true;
}

/*
 * Listen to the sequence and store it in the static array sequ
 */
void record_sequ(void){

	int8_t old_freq = 44;

	while (!sequ_ended()){
		wait_for_next_peak(old_freq);
		sequ[sequ_size] =  frequ;
		sequ_size++;
		old_freq = frequ;
//		chprintf((BaseSequentialStream *) &SD3, "frequ %d : %d\n",sequ_size,sequ[sequ_size-1]);
	}
	sequ_size -= 4;
	for (uint8_t i = 0; i < sequ_size; ++i){
		sequ[i] = ((sequ[i]+5)/3-6);
	}
}


/*
 * Starts ThdGetAudioSeq thread
 */
void audioSeq_start(void){
	chThdCreateStatic(waThdGetAudioSeq, sizeof(waThdGetAudioSeq), NORMALPRIO, ThdGetAudioSeq, NULL);
}

/*
*	Simple function used to detect the highest value in a buffer
*/
void set_peak(float* data){
	float max_norm = MIN_VALUE_THRESHOLD;
	int8_t max_norm_index = NO_PEAK;

	//search for the highest frequ
	for(uint8_t i = MIN_FREQ ; i <= MAX_FREQ ; i++){
		if(data[i] > max_norm){
			max_norm = data[i];
			max_norm_index = i;
		}
	}
	frequ = max_norm_index;
}

/*
 * To export the position sequence
 */
void get_sequ(uint8_t *sequ_size_out, int8_t *sequ_out){
	*sequ_size_out = sequ_size;
	for (uint8_t i = 0; i < sequ_size; ++i){
		sequ_out[i] = sequ[i];
	}
}


/*
*	Callback called when the demodulation of the four microphones is done.
*	We get 160 samples per mic every 10ms (16kHz)
*/
void processAudioData(int16_t *data, uint16_t num_samples){

	static uint16_t nb_samples = 0;

	//loop to fill the buffers
	for(uint16_t i = 0 ; i < num_samples ; i+=4){
		//construct an array of complex numbers. Put 0 to the imaginary part
		micFront_cmplx_input[nb_samples] = (float)data[i + MIC_FRONT];

		nb_samples++;

		micFront_cmplx_input[nb_samples] = 0;

		nb_samples++;

		//stop when buffer is full
		if(nb_samples >= (2 * FFT_SIZE)){
			break;
		}
	}

	if(nb_samples >= (2 * FFT_SIZE)){

		//FFT processing
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, micFront_cmplx_input, 0, 1);

		//Magnitude processing
		arm_cmplx_mag_f32(micFront_cmplx_input, micFront_output, FFT_SIZE);

		nb_samples = 0;

		set_peak(micFront_output);
	}
}

void wait_sequ_aquired(void){
	chBSemWait(&sequAquired);
}

