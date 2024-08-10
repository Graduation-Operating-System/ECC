#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t x;
    uint32_t y;
} Point;

#define P 0xFFFFFFFF // 소수
#define A 0 // 타원 곡선 파라미터
#define B 7 // 타원 곡선 파라미터
#define AES_BLOCK_SIZE 16

// 타원 곡선 점 추가
Point point_add(Point P1, Point P2) {
    Point R;
    if (P1.x == 0 && P1.y == 0) return P2; // P1이 무한대일 때
    if (P2.x == 0 && P2.y == 0) return P1; // P2가 무한대일 때

    uint32_t m = (P2.y - P1.y) * mod_inverse(P2.x - P1.x, P) % P; // 기울기

    R.x = (m * m - P1.x - P2.x) % P;
    R.y = (m * (P1.x - R.x) - P1.y) % P;
    return R;
}

// 스칼라 곱셈
Point scalar_multiply(Point P, uint32_t k) {
    Point R = {0, 0}; // 무한대
    Point Q = P;

    while (k) {
        if (k & 1) R = point_add(R, Q);
        Q = point_add(Q, Q); // 점 두 배
        k >>= 1;
    }
    return R;
}

// 모듈로 역수 계산
uint32_t mod_inverse(uint32_t a, uint32_t p) {
    uint32_t t = 0, new_t = 1;
    uint32_t r = p, new_r = a;

    while (new_r != 0) {
        uint32_t quotient = r / new_r;
        t = new_t;
        new_t = t - quotient * new_t;
        r = new_r;
        new_r = r - quotient * new_r;
    }
    if (r > 1) return 0; // 역수가 존재하지 않음
    if (t < 0) t += p;

    return t;
}

// AES 암호화 (간단한 XOR 대체)
void aes_encrypt(const uint8_t *input, uint8_t *output, const uint8_t *key) {
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        output[i] = input[i] ^ key[i]; // XOR 연산으로 암호화
    }
}

// AES CFB 모드 암호화
void aes_cfb_encrypt(const uint8_t *plaintext, uint8_t *ciphertext, const uint8_t *key, uint8_t *iv, size_t length) {
    uint8_t block[AES_BLOCK_SIZE];
    memcpy(block, iv, AES_BLOCK_SIZE);

    for (size_t i = 0; i < length; i++) {
        uint8_t temp[AES_BLOCK_SIZE];
        aes_encrypt(block, temp, key); // IV를 암호화

        ciphertext[i] = plaintext[i] ^ temp[0]; // 평문과 암호화된 블록 XOR
        // 블록을 시프트
        for (int j = 0; j < AES_BLOCK_SIZE - 1; j++) {
            block[j] = block[j + 1];
        }
        block[AES_BLOCK_SIZE - 1] = ciphertext[i]; // 마지막 바이트에 암호문 업데이트
    }
}

// 메인 함수
int main() {
    Point G = {1, 2}; // 타원 곡선 위의 점
    uint32_t k = 3; // 개인 키
    Point public_key = scalar_multiply(G, k); // 공개 키

    // AES CFB 암호화
    uint8_t key[AES_BLOCK_SIZE] = "1234567890123456"; // 16바이트 키
    uint8_t plaintext[] = "Hello, World!!!"; // 16바이트 평문
    uint8_t ciphertext[sizeof(plaintext)];
    uint8_t iv[AES_BLOCK_SIZE] = "0000000000000000"; // 초기화 벡터

    aes_cfb_encrypt(plaintext, ciphertext, key, iv, sizeof(plaintext));

    printf("Public Key: (%u, %u)\n", public_key.x, public_key.y);
    printf("Ciphertext: ");
    for (size_t i = 0; i < sizeof(ciphertext); i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");

    return 0;
}
