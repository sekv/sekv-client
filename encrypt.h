//#include <openssl/bio.h>
//#include <openssl/evp.h>
#include <stdint.h>
void my_aes_gcm_encrypt(char *p_src, uint32_t src_len, char *p_dst, uint32_t *dst_len, unsigned char *p_out_mac);
void my_aes_gcm_decrypt(char *p_src, uint32_t src_len, char *p_dst, uint32_t *dst_len, unsigned char *p_out_mac);
