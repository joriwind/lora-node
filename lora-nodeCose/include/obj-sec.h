#ifndef __OBJ_SEC_H__
#define __OBJ_SEC_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void objsec_init();
void objsec_set_key(uint8_t * k);

/**
 * Used to abstract the cose encryption/ key management
 * @param buffer    Buffer to push the ciphertext into
 * @param message   Input message, to be encrypted
 * @param len       Length of message
 * @param key       The key to be used
 * @param szKey     Length of the key
 * @param prefsz    
 */
size_t encrypt(uint8_t *buffer, uint16_t prefsz, const uint8_t *message, size_t len);

#ifdef __cplusplus
}
#endif

#endif

