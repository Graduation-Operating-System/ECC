#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/bn.h>

#define AES_KEY_SIZE 256
#define AES_BLOCK_SIZE 16

// ECC 키 생성 및 AES 키 반환
unsigned char* generateAESKey() {
    EC_KEY *ecc_key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_generate_key(ecc_key);

    const BIGNUM *private_key = EC_KEY_get0_private_key(ecc_key);
    unsigned char *aes_key = malloc(AES_KEY_SIZE / 8);
    
    // ECC 개인 키를 AES 키로 변환
    BN_bn2bin(private_key, aes_key);

    EC_KEY_free(ecc_key);
    return aes_key;
}

// AES 암호화 (CTR 모드)
void aes_encrypt_ctr(unsigned char *plaintext, unsigned char *ciphertext, unsigned char *key) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char counter[AES_BLOCK_SIZE];
    unsigned char output[AES_BLOCK_SIZE];
    
    // IV 및 카운터 초기화
    RAND_bytes(iv, AES_BLOCK_SIZE);
    memcpy(counter, iv, AES_BLOCK_SIZE);

    // CTR 모드 초기화
    EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, counter);

    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, strlen((char *)plaintext));
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    
    EVP_CIPHER_CTX_free(ctx);
}

// 메인 함수
int main() {
    // 평문
    unsigned char *plaintext = (unsigned char *)"Hello, World!";
    unsigned char *ciphertext = malloc(strlen((char *)plaintext) + AES_BLOCK_SIZE);
    
    // AES 키 생성
    unsigned char *aes_key = generateAESKey();

    // 암호화
    aes_encrypt_ctr(plaintext, ciphertext, aes_key);
    
    // 결과 출력
    printf("Ciphertext (CTR mode): ");
    for (int i = 0; i < strlen((char *)plaintext) + AES_BLOCK_SIZE; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");

    // 메모리 해제
    free(aes_key);
    free(ciphertext);
    return 0;
}

