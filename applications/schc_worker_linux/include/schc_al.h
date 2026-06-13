#ifndef SCHC_AL_H
#define SCHC_AL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <fullsdknet.h>

typedef enum schc_al_process_status {
    SEND_DOWN_OK,
    SEND_DOWN_BUSY,
    SEND_DOWN_OSCORE_ERROR,
    SEND_DOWN_INTERNAL_ERROR
} schc_al_process_status_t;

/**
 * @brief Initializes the SCHC Adaptation Layer.
 *
 * This function sets up the TUN device, configures its IP address,
 * and establishes routes. It also registers a handler for TUN device input.
 *
 * @return 0 on success, -1 on failure.
 */
int schc_al_init(void);

/**
 * @brief Sets the flag indicating whether processing is required.
 *
 * @param required True if processing is required, false otherwise.
 */
void schc_al_set_processing_required(bool required);

/**
 * @brief Checks if processing is required.
 *
 * @return True if processing is required, false otherwise.
 */
bool schc_al_is_processing_required(void);

/**
 * @brief Processes events for the SCHC Adaptation Layer.
 *
 * This function handles incoming IP packets from the TUN device and
 * forwards them for further processing (e.g., OSCORE encryption/decryption).
 *
 * @return 0 on success, -1 on failure.
 */
schc_al_process_status_t schc_al_process(void);

/**
 * @brief Retrieves the network callbacks for the SCHC Adaptation Layer.
 *
 * @return A pointer to the net_callbacks_t structure.
 */
const net_callbacks_t *schc_al_get_net_callbacks(void);

/**
 * @brief Terminates the SCHC Adaptation Layer.
 *
 * This function cleans up the TUN device and associated resources.
 *
 * @return 0 on success, -1 on failure.
 */
int schc_al_terminate(void);

#endif // SCHC_AL_H
