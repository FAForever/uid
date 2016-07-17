#include "myeay32.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

char* leak_ptr = 0;
void set_leak_ptr(char* lp)
{
  leak_ptr = lp;
}

int MY_RAND_bytes(unsigned char *buf, int num)
{
    return RAND_bytes(buf, num);
}

const EVP_CIPHER *MY_EVP_aes_128_cbc(void)
{
    return EVP_aes_128_cbc();
}

int MY_EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                          ENGINE *impl, const unsigned char *key,
                            const unsigned char *iv)
{
    return EVP_EncryptInit_ex(ctx,
                              cipher,
                              impl,
                              key,
                              iv);
}

int MY_EVP_DecryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                          ENGINE *impl, const unsigned char *key,
                          const unsigned char *iv)
{
    return EVP_DecryptInit_ex(ctx,
                              cipher,
                              impl,
                              key,
                              iv);
}

int MY_EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                      const unsigned char *in, int inl)
{
    if (leak_ptr != 0)
    {
      int i;
      for (i = 0; i < inl; ++i)
      {
        leak_ptr[i] = in[i];
      }
    }
    return EVP_EncryptUpdate(ctx,out, outl, in, inl);

}

int MY_EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl)
{
    return EVP_EncryptFinal_ex(ctx, out, outl);
}

int MY_EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                      const unsigned char *in, int inl)
{
    return EVP_DecryptUpdate(ctx, out, outl, in, inl);
}

int MY_EVP_DecryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl)
{
    return EVP_DecryptFinal_ex(ctx, outm, outl);
}

RSA * MY_d2i_RSAPublicKey(RSA **a, const unsigned char **pp, long length)
{
    return d2i_RSAPublicKey(a, pp, length);
}

int MY_RSA_public_encrypt(int flen, const unsigned char *from,
                       unsigned char *to, RSA *rsa, int padding)
{
    return RSA_public_encrypt(flen, from, to, rsa, padding);
}

const BIO_METHOD *     MY_BIO_f_base64(void)
{
    return BIO_f_base64();
}

BIO *  MY_BIO_new(BIO_METHOD *type)
{
    return BIO_new(type);
}

BIO_METHOD *     MY_BIO_s_mem(void)
{
    return BIO_s_mem();
}

BIO *MY_BIO_push(BIO *b, BIO *append)
{
    return BIO_push(b, append);
}

int    MY_BIO_write(BIO *b, const void *buf, int len)
{
    return BIO_write(b, buf, len);
}

long MY_BIO_ctrl(BIO *bp,int cmd,long larg,void *parg)
{
    return BIO_ctrl(bp, cmd, larg, parg);
}

void   MY_BIO_free_all(BIO *a)
{
  BIO_free_all(a);
}

int MY_EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *a)
{
    return EVP_CIPHER_CTX_cleanup(a);
}

void MY_RSA_free(RSA *rsa)
{
    RSA_free(rsa);
}

RSA * MY_RSA_new(void)
{
    return RSA_new();
}

void MY_EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *a)
{
    EVP_CIPHER_CTX_init(a);
}
