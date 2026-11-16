/*
 * app_ota_service.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: OTA升级服务逻辑
 *
 *  升级包是带地址的加密 hex，只有 A 面和 B 面，没有 Boot。
 *  上位机按地址把整份发出去，不看这次升哪一面。
 *  标志指定升 A：只解密并写入落在 A 区的数据；B 区的包不写，直接回成功。
 *  标志指定升 B：A 区的包直接回成功；只解密并写入落在 B 区的数据。
 *  A、B 各自从同一个 IV 做 AES-CBC，只解一面时从这一面的第一块算起。
 */
#ifndef FW_APP_06_APP_OTA_SERVICE_APP_OTA_SERVICE_H_
#define FW_APP_06_APP_OTA_SERVICE_APP_OTA_SERVICE_H_



#endif /* FW_APP_06_APP_OTA_SERVICE_APP_OTA_SERVICE_H_ */
