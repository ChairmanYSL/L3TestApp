#ifndef __DLL_PAYPASS_AES_H__
#define __DLL_PAYPASS_AES_H__

#define AES_ENCRYPT	1
#define AES_DECRYPT	0

#define ECB_TYPE  0
#define CBC_TYPE  1


#ifndef S8
    #define S8 char
#endif

#ifndef S16
    #define S16 short
#endif

#ifndef S32
    #define S32 int
#endif

#ifndef U8
    #define U8 unsigned char
#endif

#ifndef U16
    #define U16 unsigned short
#endif

#ifndef U32
    #define U32 unsigned int
#endif

#ifndef s8
    #define s8 char
#endif

#ifndef s16
    #define s16 short
#endif

#ifndef s32
    #define s32 int
#endif

#ifndef u8
    #define u8 unsigned char
#endif

#ifndef u16
    #define u16 unsigned short
#endif

#ifndef u32
    #define u32 unsigned int
#endif


#define AES_MAXNR 14

struct aes_key_st
{
    u32 rd_key[4 * (AES_MAXNR + 1)];
    s32 rounds;
};
typedef struct aes_key_st AES_KEY;


extern s32 dllpaypassaes_encrypt(u8 *plain, u32 len, u8 *cipher, u8 *key, u8 keylen, u8 mode, u8 *initvector);
extern s32 dllpaypassaes_decrypt (u8 *cipher, u32 len, u8 *plain, u8 *key, u8 keylen, u8 mode, u8 *initvector);

#endif

