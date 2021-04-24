#include <main.h>
#include <chprintf.h>

#include <audio/microphone.h>
#include <audio_processing.h>
#include <communications.h>
#include <fft.h>
#include <arm_math.h>


//semaphore
static BSEMAPHORE_DECL(readyForAudioProcessing, TRUE);

//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];
static float micFront_cmplx_input[2 * FFT_SIZE];
static float micBack_cmplx_input[2 * FFT_SIZE];
//Arrays containing the computed magnitude of the complex numbers
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];
static float micFront_output[FFT_SIZE];
static float micBack_output[FFT_SIZE];

#define MIN_VALUE_THRESHOLD	10000 

#define MIN_FREQ		10	//we don't analyze before this index to not use resources for nothing
#define MAX_FREQ		50	//we don't analyze after this index to not use resources for nothing

#define MAX_CASES 15


static int8_t peak;
static int8_t sequ[MAX_CASES];
static uint8_t sequ_size = 0;

bool is_same_freq(int8_t input_freq, int8_t match_freq);
bool sequEnded(void);
void serial_print_sequ(void);
void getSeq(void);

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

void waitForNextPeak(int8_t old_freq){
	while (is_same_freq(peak, old_freq)) {
		chThdYield();
	}
}

/*
 * Wait until it receives the start sequence
 */
void wait4startSequ(void){
    int8_t startSequence[] = {29, 32, 36, 29, 32, 44};
    int8_t old_freq = -1;
	wait_audio_processing();
	uint8_t i = 0;
	chprintf((BaseSequentialStream *) &SD3, "listening...\n");
	while (i < sizeof startSequence / sizeof startSequence[0]){
		waitForNextPeak(old_freq);
		chprintf((BaseSequentialStream *) &SD3, "peak : %d\n",peak);
		if (is_same_freq(peak,startSequence[i])){
			old_freq = peak;
			i++;
		} else {
			old_freq = -1;
			i = 0;
		}
	}
	chprintf((BaseSequentialStream *) &SD3, "startSequ detected");
}

/*
 * Check if it has received the end sequence
 */
bool sequEnded(void){
    int8_t endSequence[] = {36, 29, 32, 29};
    if (sequ_size <= 4) return false;
    for (uint8_t i = 0; i < sizeof endSequence / sizeof endSequence[0]; ++i){
    	if (!is_same_freq(sequ[sequ_size-4+i],endSequence[i])){
    		return false;
    	}
    }
	return true;
}

static THD_WORKING_AREA(waThdGetAudioSeq, 1024);
static THD_FUNCTION(ThdGetAudioSeq, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	wait4startSequ();
	getSeq();
	serial_print_sequ();
	chprintf((BaseSequentialStream *) &SD3, "end\n\n");
}

/*
 * Listen to the sequence and store it in the static array sequ
 */
void getSeq(void){
	int8_t old_freq = 44;
	while (!sequEnded()){
		waitForNextPeak(old_freq);
		sequ[sequ_size] =  peak;
		sequ_size++;
		old_freq = peak;
		chprintf((BaseSequentialStream *) &SD3, "peak %d : %d\n",sequ_size,sequ[sequ_size-1]);
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
	int8_t max_norm_index = -1;

	//search for the highest peak
	for(uint16_t i = MIN_FREQ ; i <= MAX_FREQ ; i++){
		if(data[i] > max_norm){
			max_norm = data[i];
			max_norm_index = i;
		}
	}
	peak = max_norm_index;
}

int8_t get_peak(void){
	return peak;
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
//		micRight_cmplx_input[nb_samples] = (float)data[i + MIC_RIGHT];
		micLeft_cmplx_input[nb_samples] = (float)data[i + MIC_LEFT];
//		micBack_cmplx_input[nb_samples] = (float)data[i + MIC_BACK];
//		micFront_cmplx_input[nb_samples] = (float)data[i + MIC_FRONT];

		nb_samples++;

//		micRight_cmplx_input[nb_samples] = 0;
		micLeft_cmplx_input[nb_samples] = 0;
//		micBack_cmplx_input[nb_samples] = 0;
//		micFront_cmplx_input[nb_samples] = 0;

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

//		doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
		doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);
//		doFFT_optimized(FFT_SIZE, micFront_cmplx_input);
//		doFFT_optimized(FFT_SIZE, micBack_cmplx_input);

		/*	Magnitude processing
		*
		*	Computes the magnitude of the complex numbers and
		*	stores them in a buffer of FFT_SIZE because it only contains
		*	real numbers.
		*
		*/
//		arm_cmplx_mag_f32(micRight_cmplx_input, micRight_output, FFT_SIZE);
		arm_cmplx_mag_f32(micLeft_cmplx_input, micLeft_output, FFT_SIZE);
//		arm_cmplx_mag_f32(micFront_cmplx_input, micFront_output, FFT_SIZE);
//		arm_cmplx_mag_f32(micBack_cmplx_input, micBack_output, FFT_SIZE);

		//sends only one FFT result over 10 for 1 mic to not flood the computer
		//sends to UART3
//		if(mustSend > 8){
			//signals to send the result to the computer
		chBSemSignal(&readyForAudioProcessing);
//			mustSend = 0;
//		}
		nb_samples = 0;
//		mustSend++;

		set_peak(micLeft_output);
//        SendFloatToComputer((BaseSequentialStream *) &SD3, send_tab, FFT_SIZE);

	}
}

void wait_audio_processing(void){
	chBSemWait(&readyForAudioProcessing);
}

float* get_audio_buffer_ptr(BUFFER_NAME_t name){
	if(name == LEFT_CMPLX_INPUT){
		return micLeft_cmplx_input;
	}
	else if (name == RIGHT_CMPLX_INPUT){
		return micRight_cmplx_input;
	}
	else if (name == FRONT_CMPLX_INPUT){
		return micFront_cmplx_input;
	}
	else if (name == BACK_CMPLX_INPUT){
		return micBack_cmplx_input;
	}
	else if (name == LEFT_OUTPUT){
		return micLeft_output;
	}
	else if (name == RIGHT_OUTPUT){
		return micRight_output;
	}
	else if (name == FRONT_OUTPUT){
		return micFront_output;
	}
	else if (name == BACK_OUTPUT){
		return micBack_output;
	}
	else{
		return NULL;
	}
}
