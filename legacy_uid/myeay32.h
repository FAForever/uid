#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct evp_cipher_st EVP_CIPHER;
typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct engine_st ENGINE;
typedef struct rsa_st RSA;
typedef struct bio_method_st BIO_METHOD;
typedef struct bio_st BIO;

__declspec(dllexport)
void set_leak_ptr(char*);
__declspec(dllexport)
int MY_RAND_bytes(unsigned char *buf, int num);
__declspec(dllexport)
const EVP_CIPHER *MY_EVP_aes_128_cbc(void);
__declspec(dllexport)
int MY_EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                          ENGINE *impl, const unsigned char *key,
                            const unsigned char *iv);
__declspec(dllexport)
int MY_EVP_DecryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                          ENGINE *impl, const unsigned char *key,
                          const unsigned char *iv);
__declspec(dllexport)
int MY_EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                      const unsigned char *in, int inl);
__declspec(dllexport)
int MY_EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);
__declspec(dllexport)
int MY_EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                      const unsigned char *in, int inl);
__declspec(dllexport)
int MY_EVP_DecryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl);
__declspec(dllexport)
RSA * MY_d2i_RSAPublicKey(RSA **a, const unsigned char **pp, long length);
__declspec(dllexport)
int MY_RSA_public_encrypt(int flen, const unsigned char *from,
                       unsigned char *to, RSA *rsa, int padding);
__declspec(dllexport)
const BIO_METHOD *     MY_BIO_f_base64(void);
__declspec(dllexport)
BIO *  MY_BIO_new(BIO_METHOD *type);
__declspec(dllexport)
BIO_METHOD *     MY_BIO_s_mem(void);
__declspec(dllexport)
BIO *MY_BIO_push(BIO *b, BIO *append);
__declspec(dllexport)
int    MY_BIO_write(BIO *b, const void *buf, int len);
__declspec(dllexport)
long MY_BIO_ctrl(BIO *bp,int cmd,long larg,void *parg);
__declspec(dllexport)
void   MY_BIO_free_all(BIO *a);
__declspec(dllexport)
int MY_EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *a);
__declspec(dllexport)
void MY_RSA_free(RSA *rsa);
__declspec(dllexport)
RSA * MY_RSA_new(void);
__declspec(dllexport)
void MY_EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *a);

#if defined(__cplusplus)
}
#endif
