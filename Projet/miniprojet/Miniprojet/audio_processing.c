#include <main.h>
#include <chprintf.h>

#include <audio/microphone.h>
#include <audio_processing.h>
#include <fft.h>
#include <arm_math.h>


//semaphore
static BSEMAPHORE_DECL(sequAquired, TRUE);

//2 times FFT_SIZE because this array contain complex numbers (real + imaginary)
static float micFront_cmplx_input[2 * FFT_SIZE];
//Array containing the computed magnitude of the complex numbers
static float micFront_output[FFT_SIZE];

#define MIN_VALUE_THRESHOLD	10000 

#define MIN_FREQ		10	//we don't analyze before this index to not use resources for nothing
#define MAX_FREQ		50	//we don't analyze after this index to not use resources for nothing

#define MAX_CASES 15
#define NO_PEAK 255

static uint8_t frequ;
//static uint8_t sequ[MAX_CASES];
//static uint8_t sequ_size = 0;
static uint8_t sequ[] = {1,4,7,4,1};
static uint8_t sequ_size = 5;

bool is_same_freq(int8_t input_freq, int8_t match_freq);
bool sequ_ended(void);
void serial_print_sequ(void);
void record_sequ(void);

/*
 * Print the received sequence over serial
 */
void serial_print_sequ(void){
	chprintf((BaseSequentialStream *) &SD3, "\nSequ: [ ");
	for (uint i = 0; i < sequ_size; ++i){
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
    uint8_t startSequence[] = {29, 32, 36, 29, 32, 44};
    uint8_t old_freq = NO_PEAK;
	uint8_t i = 0;
	chprintf((BaseSequentialStream *) &SD3, "listening...\n");
	while (i < sizeof startSequence / sizeof startSequence[0]){
		wait_for_next_peak(old_freq);
		chprintf((BaseSequentialStream *) &SD3, "frequ : %d\n",frequ);
		if (is_same_freq(frequ,startSequence[i])){
			old_freq = frequ;
			i++;
		} else {
			old_freq = NO_PEAK;
			i = 0;
		}
	}
	chprintf((BaseSequentialStream *) &SD3, "startSequ detected");
}

/*
 * Check if it has received the end sequence
 */
bool sequ_ended(void){
    uint8_t endSequence[] = {36, 29, 32, 29};
    if (sequ_size <= 4) return false;
    for (uint8_t i = 0; i < sizeof endSequence / sizeof endSequence[0]; ++i){
    	if (!is_same_freq(sequ[sequ_size-4+i], endSequence[i])){
    		return false;
    	}
    }
	return true;
}

static THD_WORKING_AREA(waThdGetAudioSeq, 1024);
static THD_FUNCTION(ThdGetAudioSeq, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

//	wait_for_start_sequ();
//	record_sequ();
//	serial_print_sequ();

	chprintf((BaseSequentialStream *) &SD3, "end\n\n");
	chBSemSignal(&sequAquired);
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
		chprintf((BaseSequentialStream *) &SD3, "frequ %d : %d\n",sequ_size,sequ[sequ_size-1]);
	}
	sequ_size -= 4;
	for (uint8_t i = 0; i < sequ_size; ++i){
		sequ[i] = ((sequ[i]+5)/3-6);
	}
}

void audioSeq_start(void){
	chThdCreateStatic(waThdGetAudioSeq, sizeof(waThdGetAudioSeq), NORMALPRIO, ThdGetAudioSeq, NULL);
}


/*
*	Simple function used to detect the highest value in a buffer
*/
void set_peak(float* data){
	float max_norm = MIN_VALUE_THRESHOLD;
	uint8_t max_norm_index = NO_PEAK;

	//search for the highest frequ
	for(uint8_t i = MIN_FREQ ; i <= MAX_FREQ ; i++){
		if(data[i] > max_norm){
			max_norm = data[i];
			max_norm_index = i;
		}
	}
	frequ = max_norm_index;
}


void get_sequ(uint8_t *sequ_size_out, uint8_t *sequ_out){
	*sequ_size_out = sequ_size;
	for (uint8_t i = 0; i < sequ_size; ++i){
		sequ_out[i] = sequ[i];
	}
}


/*
*	Callback called when the demodulation of the four microphones is done.
*	We get 160 samples per mic every 10ms (16kHz)
*	
*	params :
*	int16_t *data			Buffer containing 4 times 160 samples. the samples are sorted by micro
*							so we have [micRight1, micLeft1, micBack1, micFront1, micRight2, etc...]
*	uint16_t num_samples	Tells how many data we get in total (should always be 640)
*/
void processAudioData(int16_t *data, uint16_t num_samples){

	/*
	*
	*	We get 160 samples per mic every 10ms
	*	So we fill the samples buffers to reach
	*	1024 samples, then we compute the FFTs.
	*
	*/

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
		/*	FFT proccessing
		*
		*	This FFT function stores the results in the input buffer given.
		*	This is an "In Place" function. 
		*/

		doFFT_optimized(FFT_SIZE, micFront_cmplx_input);

		/*	Magnitude processing
		*
		*	Computes the magnitude of the complex numbers and
		*	stores them in a buffer of FFT_SIZE because it only contains
		*	real numbers.
		*
		*/
		arm_cmplx_mag_f32(micFront_cmplx_input, micFront_output, FFT_SIZE);


		nb_samples = 0;

		set_peak(micFront_output);
	}
}

void wait_sequ_aquired(void){
	chBSemWait(&sequAquired);
}

