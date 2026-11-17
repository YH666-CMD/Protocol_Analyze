#include "ota_boot.h"
#include "config.h"
#include "ota_def.h"

#if (APP_SLOT == 0)
#define IMG_LINK_OFS    MEM_SLOT_A_OFS
#elif (APP_SLOT == 1)
#define IMG_LINK_OFS    MEM_SLOT_B_OFS
#endif

extern char _img_end[];

/* 没有别的代码引用它。链接脚本 KEEP 这一段，避免被丢掉。 */
const img_hdr_t g_img_hdr __attribute__((section(".img_hdr"), used)) =
{
    MEM_IMG_MAGIC,
    IMG_LINK_OFS,
    FW_VERSION,
    (uint32_t)_img_end
};
