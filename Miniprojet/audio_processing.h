#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H


#define FFT_SIZE 	1024
#define MAX_MOVES 	20
typedef enum {
	//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
	LEFT_CMPLX_INPUT = 0,
	RIGHT_CMPLX_INPUT,
	FRONT_CMPLX_INPUT,
	BACK_CMPLX_INPUT,
	//Arrays containing the computed magnitude of the complex numbers
	LEFT_OUTPUT,
	RIGHT_OUTPUT,
	FRONT_OUTPUT,
	BACK_OUTPUT
} BUFFER_NAME_t;


/**
 * @brief starts the GetAudioSeq thread (to be used in main)
 */
void audioSeq_start(void);

/**
 * @brief	places the position array in sequ_out
 *
 * @param 	sequ_size_out	size of array
 * @param	sequ_out 		the array itself
 */
void get_sequ(uint8_t *sequ_size_out, int8_t *sequ_out);

/**
 * @brief	processes the audio data by using fft
 *
 * @param 	data			Buffer containing 4 times 160 samples. the samples are sorted by micro
*							so we have [micRight1, micLeft1, micBack1, micFront1, micRight2, etc...]
 * @param	num_samples		Tells how many data we get in total (should always be 640)
 */
void processAudioData(int16_t *data, uint16_t num_samples);

/**
* @brief	put the invoking thread into sleep until it can process the audio datas
*/
void  wait_sequ_aquired(void);

#endif /* AUDIO_PROCESSING_H */
