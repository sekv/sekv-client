#include "encrypt.h"
#include <openssl/bio.h>
#include <openssl/evp.h>

void my_aes_gcm_encrypt(char *p_src, uint32_t src_len, char *p_dst, uint32_t *dst_len, unsigned char *p_out_mac){
    uint8_t gcm_key[16]= {
    0xee,0xbc,0x1f,0x57,0x48,0x7f,0x51,0x92,0x1c,0x04,0x65,0x66,
    0x5f,0x8a,0xe6,0xd1
    };
    uint8_t gcm_iv[12] = {
    0x99,0xaa,0x3e,0x68,0xed,0x81,0x73,0xa0,0xee,0xd0,0x66,0x84
    };

    EVP_CIPHER_CTX *ctx;
//    sgx_aes_gcm_128bit_tag_t *p_out_mac;
//    p_out_mac = (sgx_aes_gcm_128bit_tag_t *)malloc(sizeof(sgx_aes_gcm_128bit_tag_t)*1000);
//    unsigned char *p_out_mac;
//    p_out_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);
//    printf("enter aes gcm\n");
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(gcm_iv), NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, gcm_key, gcm_iv);
//    printf("aes gcm init success!\n");
    EVP_EncryptUpdate(ctx, p_dst, dst_len, p_src, strlen(p_src));
//    printf("aes gcm update\n");
//        printf("Ciphertext:\n");
//        BIO_dump_fp(stdout, p_dst, *dst_len);
//        printf("dst_len:%d\n",*dst_len);
    EVP_EncryptFinal_ex(ctx, p_dst, dst_len);
//    printf("aes gcm final\n");
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, p_out_mac);
//    printf("aes gcm mac\n");
    EVP_CIPHER_CTX_free(ctx);


//        printf("Ciphertext:\n");
//        BIO_dump_fp(stdout, p_dst, strlen(p_dst));
//        printf("dst_len:%d\n",*dst_len);
}

void my_aes_gcm_decrypt(char *p_src, uint32_t src_len, char *p_dst, uint32_t *dst_len, unsigned char *p_out_mac){
    uint8_t gcm_key[16]= {
    0xee,0xbc,0x1f,0x57,0x48,0x7f,0x51,0x92,0x1c,0x04,0x65,0x66,
    0x5f,0x8a,0xe6,0xd1
    };
    uint8_t gcm_iv[12] = {
    0x99,0xaa,0x3e,0x68,0xed,0x81,0x73,0xa0,0xee,0xd0,0x66,0x84
    };

    EVP_CIPHER_CTX *ctx;
//    sgx_aes_gcm_128bit_tag_t *p_out_mac;
//    p_out_mac = (sgx_aes_gcm_128bit_tag_t *)malloc(sizeof(sgx_aes_gcm_128bit_tag_t)*1000);
//    unsigned char *p_out_mac;
//    p_out_mac = (unsigned char *)malloc(sizeof(unsigned char)*16);

    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(gcm_iv), NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, gcm_key, gcm_iv);
    EVP_DecryptUpdate(ctx, p_dst, dst_len, p_src, strlen(p_src));

//        printf("Plaintext:\n");
//        BIO_dump_fp(stdout, p_dst, *dst_len);
//        printf("dst_len:%d\n",*dst_len);
    EVP_DecryptFinal_ex(ctx, p_dst, dst_len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, p_out_mac);
    EVP_CIPHER_CTX_free(ctx);


        printf("Plaintext:\n");
        BIO_dump_fp(stdout, p_dst, strlen(p_dst));
        printf("dst_len:%d\n",*dst_len);
}

