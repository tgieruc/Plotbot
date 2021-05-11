#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

/**
 * @brief	returns the position in pixels of the widest black line
 */
uint16_t get_position_px(void);

/**
 * @brief	starts the ProcessImage and CapureImage threads
 */
void process_image_start(void);

/**
 * @brief	waits for the position_ready_sem semaphore to be activated
 */
void wait_position_acquired(void);

#endif /* PROCESS_IMAGE_H */
