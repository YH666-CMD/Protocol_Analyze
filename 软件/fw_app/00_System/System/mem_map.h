/*
 * mem_map.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: Flash 和 RAM 的分区。只有数字和检查，没有属性。
 *                    链接脚本以后也预处理这份头。
 *
 *  使用前先定义 APP_SLOT：0 是 A 面，1 是 B 面。
 *  零等待区用别名 0x0000xxxx 链接和取指，擦写用物理地址 0x0800xxxx。
 *  非零等待区没有低地址别名，偏移从 0x10000 起，读写都用物理地址。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_MEM_MAP_H_
#define FW_APP_00_SYSTEM_SYSTEM_MEM_MAP_H_

#if defined(MEM_LD)
#define MEM_U(x) (x)            /* 链接脚本不认识 u 后缀 */
#else
#define MEM_U(x) ((x##u))
#endif

#define MEM_FLASH_PHY_BASE     MEM_U(0x08000000)   /* 擦写寄存器用 */
#define MEM_FLASH_RUN_BASE     MEM_U(0x00000000)   /* 零等待区的链接、取指别名 */
#define MEM_FLASH_SIZE         (MEM_U(64) * MEM_U(1024))   /* 零等待区 */
#define MEM_FLASH_TOTAL        (MEM_U(224) * MEM_U(1024))  /* 含数据区 */
#define MEM_FLASH_PAGE         MEM_U(256)
#define MEM_FLASH_SECTOR       MEM_U(4096)
#define MEM_FLASH_ERASED_WORD  MEM_U(0xE339E339)

#define MEM_BOOT_OFS           MEM_U(0x0000)
#define MEM_BOOT_AREA_SIZE     (MEM_U(2) * MEM_U(1024))
#define MEM_BOOT_CODE_SIZE     MEM_BOOT_AREA_SIZE
#define MEM_SLOT_SIZE          (MEM_U(30) * MEM_U(1024))
#define MEM_SLOT_A_OFS         (MEM_BOOT_OFS + MEM_BOOT_AREA_SIZE)
#define MEM_SLOT_B_OFS         (MEM_SLOT_A_OFS + MEM_SLOT_SIZE)
#define MEM_FLAG_OFS           (MEM_SLOT_B_OFS + MEM_SLOT_SIZE)
#define MEM_FLAG_SIZE          (MEM_U(2) * MEM_U(1024))

/* 下面都是相对 0x08000000 的偏移，没有 0x0000 别名 */
#define MEM_NZW_OFS            MEM_U(0x10000)
#define MEM_NZW_SIZE           (MEM_U(16) * MEM_U(1024))
#define MEM_PARAM_OFS          (MEM_NZW_OFS + MEM_NZW_SIZE)
#define MEM_PARAM_SIZE         (MEM_U(16) * MEM_U(1024))
#define MEM_CRASH_OFS          (MEM_PARAM_OFS + MEM_PARAM_SIZE)
#define MEM_CRASH_SIZE         (MEM_U(16) * MEM_U(1024))
#define MEM_LOG_OFS            (MEM_CRASH_OFS + MEM_CRASH_SIZE)
#define MEM_LOG_SIZE           (MEM_U(64) * MEM_U(1024))
#define MEM_FACTORY_OFS        (MEM_LOG_OFS + MEM_LOG_SIZE)
#define MEM_FACTORY_SIZE       (MEM_U(48) * MEM_U(1024))

#define MEM_RAM_BASE           MEM_U(0x20000000)
#define MEM_RAM_SIZE           (MEM_U(20) * MEM_U(1024))
#define MEM_SHARE_SIZE         MEM_U(256)
#define MEM_SHARE_ADDR         (MEM_RAM_BASE + MEM_RAM_SIZE - MEM_SHARE_SIZE)
#define MEM_STACK_SIZE         MEM_U(2048)

#define MEM_IMG_HDR_OFS        MEM_U(0x100)        /* 镜像头在本面镜像内的偏移 */

#define MEM_LD_APP_LENGTH      MEM_SLOT_SIZE

#ifdef APP_SLOT
#if (APP_SLOT == 0)
#define MEM_LD_APP_ORIGIN      (MEM_FLASH_RUN_BASE + MEM_SLOT_A_OFS)
#elif (APP_SLOT == 1)
#define MEM_LD_APP_ORIGIN      (MEM_FLASH_RUN_BASE + MEM_SLOT_B_OFS)
#else
#error "APP_SLOT 只能是 0（A 面）或 1（B 面）"
#endif
#endif

#if ((MEM_BOOT_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_BOOT_AREA_SIZE % MEM_FLASH_PAGE) != 0)
#error "Boot 区没有按 256 字节对齐"
#endif
#if ((MEM_SLOT_A_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_SLOT_SIZE % MEM_FLASH_PAGE) != 0)
#error "A 面没有按 256 字节对齐"
#endif
#if ((MEM_SLOT_B_OFS % MEM_FLASH_PAGE) != 0)
#error "B 面没有按 256 字节对齐"
#endif
#if ((MEM_FLAG_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_FLAG_SIZE % MEM_FLASH_PAGE) != 0)
#error "标志区没有按 256 字节对齐"
#endif
#if ((MEM_NZW_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_NZW_SIZE % MEM_FLASH_PAGE) != 0)
#error "慢速代码区没有按 256 字节对齐"
#endif
#if ((MEM_PARAM_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_PARAM_SIZE % MEM_FLASH_PAGE) != 0)
#error "参数区没有按 256 字节对齐"
#endif
#if ((MEM_CRASH_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_CRASH_SIZE % MEM_FLASH_PAGE) != 0)
#error "故障快照区没有按 256 字节对齐"
#endif
#if ((MEM_LOG_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_LOG_SIZE % MEM_FLASH_PAGE) != 0)
#error "日志区没有按 256 字节对齐"
#endif
#if ((MEM_FACTORY_OFS % MEM_FLASH_PAGE) != 0) || ((MEM_FACTORY_SIZE % MEM_FLASH_PAGE) != 0)
#error "出厂区没有按 256 字节对齐"
#endif
#if ((MEM_BOOT_AREA_SIZE + MEM_SLOT_SIZE + MEM_SLOT_SIZE + MEM_FLAG_SIZE) != MEM_FLASH_SIZE)
#error "零等待区四块加起来不是 64KB"
#endif
#if ((MEM_FACTORY_OFS + MEM_FACTORY_SIZE) != MEM_FLASH_TOTAL)
#error "从芯片开头到出厂区结束不是 224KB"
#endif
#if ((MEM_FACTORY_OFS + MEM_FACTORY_SIZE - MEM_NZW_OFS) + MEM_FLASH_SIZE != MEM_FLASH_TOTAL)
#error "非零等待区加零等待区不是整片 Flash"
#endif
#if (MEM_LD_APP_LENGTH != (MEM_U(30) * MEM_U(1024)))
#error "App 链接长度必须是 30KB"
#endif
#if ((MEM_SHARE_ADDR % 4u) != 0) || ((MEM_SHARE_SIZE % 4u) != 0)
#error "共享区没有按 4 字节对齐"
#endif

#endif /* FW_APP_00_SYSTEM_SYSTEM_MEM_MAP_H_ */
