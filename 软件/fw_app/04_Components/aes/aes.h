/*
 * aes.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: AES-CBC。业务层只依赖本契约，不包含芯片头文件。
 *
 * 约定：
 *   1. 每次处理 16 字节。IV 为 NULL 时不做向量异或。
 *   2. 密钥长度分别是 16、24、32 字节。
 *   3. 加密时 block 放入明文，返回后变成密文，IV 被更新成这块密文。
 *   4. 解密时 block 放入密文，返回后变成明文，IV 被更新成原来的密文。
 *   5. 轮密钥是内部静态缓冲区。加密和解密不要重入，也不要在中断里交叉调用。
 */
#ifndef FW_APP_04_COMPONENTS_AES_AES_H_
#define FW_APP_04_COMPONENTS_AES_AES_H_

#include <stdint.h>

typedef struct
{
    void (*pf_aes128_encrypt)(uint8_t *iv, uint8_t *block, const uint8_t *key);
    void (*pf_aes128_decrypt)(uint8_t *iv, uint8_t *block, const uint8_t *key);
    void (*pf_aes192_encrypt)(uint8_t *iv, uint8_t *block, const uint8_t *key);
    void (*pf_aes192_decrypt)(uint8_t *iv, uint8_t *block, const uint8_t *key);
    void (*pf_aes256_encrypt)(uint8_t *iv, uint8_t *block, const uint8_t *key);
    void (*pf_aes256_decrypt)(uint8_t *iv, uint8_t *block, const uint8_t *key);
} aes_driver_interface_t;

/* 纯软件实现，结构体放在 Flash */
extern const aes_driver_interface_t aes_sw_driver;

#endif /* FW_APP_04_COMPONENTS_AES_AES_H_ */
