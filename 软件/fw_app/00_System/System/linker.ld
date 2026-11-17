/*
 * 链接脚本模板。构建前预处理成 Ld/Link_gen.ld。
 * APP_SLOT 在 config.h：0 是 A 面，1 是 B 面。
 * 不要手改生成结果。改分区只改 mem_map.h。
 */
#include "config.h"

ENTRY( _start )

__stack_size = MEM_STACK_SIZE;

PROVIDE( _stack_size = __stack_size );


MEMORY
{
	FLASH (rx) : ORIGIN = MEM_LD_APP_ORIGIN, LENGTH = MEM_LD_APP_LENGTH
	NZW   (rx) : ORIGIN = MEM_FLASH_PHY_BASE + MEM_NZW_OFS, LENGTH = MEM_NZW_SIZE
	RAM   (xrw) : ORIGIN = MEM_RAM_BASE, LENGTH = MEM_RAM_SIZE - MEM_SHARE_SIZE
	SHARE (rw)  : ORIGIN = MEM_SHARE_ADDR, LENGTH = MEM_SHARE_SIZE
}


SECTIONS
{

	.init :
	{
		_sinit = .;
		. = ALIGN(4);
		KEEP(*(SORT_NONE(.init)))
		. = ALIGN(4);
		_einit = .;
	} >FLASH AT>FLASH

  .vector :
  {
      *(.vector);
	  . = ALIGN(64);
  } >FLASH AT>FLASH

	.img_hdr :
	{
		KEEP(*(.img_hdr))
	} >FLASH AT>FLASH

	ASSERT(ADDR(.img_hdr) == ORIGIN(FLASH) + MEM_IMG_HDR_OFS, "image header is not at +0x100")
	ASSERT(SIZEOF(.img_hdr) == 16, "image header is not 16 bytes")

	.text :
	{
		. = ALIGN(4);
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata*)
		*(.gnu.linkonce.t.*)
		. = ALIGN(4);
	} >FLASH AT>FLASH

	.fini :
	{
		KEEP(*(SORT_NONE(.fini)))
		. = ALIGN(4);
	} >FLASH AT>FLASH

	PROVIDE( _etext = . );
	PROVIDE( _eitcm = . );

	.preinit_array  :
	{
	  PROVIDE_HIDDEN (__preinit_array_start = .);
	  KEEP (*(.preinit_array))
	  PROVIDE_HIDDEN (__preinit_array_end = .);
	} >FLASH AT>FLASH

	.init_array     :
	{
	  PROVIDE_HIDDEN (__init_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
	  KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
	  PROVIDE_HIDDEN (__init_array_end = .);
	} >FLASH AT>FLASH

	.fini_array     :
	{
	  PROVIDE_HIDDEN (__fini_array_start = .);
	  KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
	  KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
	  PROVIDE_HIDDEN (__fini_array_end = .);
	} >FLASH AT>FLASH

	.ctors          :
	{
	  KEEP (*crtbegin.o(.ctors))
	  KEEP (*crtbegin?.o(.ctors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
	  KEEP (*(SORT(.ctors.*)))
	  KEEP (*(.ctors))
	} >FLASH AT>FLASH

	.dtors          :
	{
	  KEEP (*crtbegin.o(.dtors))
	  KEEP (*crtbegin?.o(.dtors))
	  KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
	  KEEP (*(SORT(.dtors.*)))
	  KEEP (*(.dtors))
	} >FLASH AT>FLASH

	.dalign :
	{
		. = ALIGN(4);
		PROVIDE(_data_vma = .);
	} >RAM AT>FLASH

	.dlalign :
	{
		. = ALIGN(4);
		PROVIDE(_data_lma = .);
	} >FLASH AT>FLASH

	.data :
	{
    	*(.gnu.linkonce.r.*)
    	*(.data .data.*)
    	*(.gnu.linkonce.d.*)
		. = ALIGN(8);
    	PROVIDE( __global_pointer$ = . + 0x800 );
    	*(.sdata .sdata.*)
		*(.sdata2.*)
    	*(.gnu.linkonce.s.*)
    	. = ALIGN(8);
    	*(.srodata.cst16)
    	*(.srodata.cst8)
    	*(.srodata.cst4)
    	*(.srodata.cst2)
    	*(.srodata .srodata.*)
    	. = ALIGN(4);
		PROVIDE( _edata = .);
	} >RAM AT>FLASH

	PROVIDE(_img_end = LOADADDR(.data) + SIZEOF(.data));
	ASSERT(_img_end <= ORIGIN(FLASH) + LENGTH(FLASH), "image end is outside the slot")

	.bss :
	{
		. = ALIGN(4);
		PROVIDE( _sbss = .);
  	    *(.sbss*)
        *(.gnu.linkonce.sb.*)
		*(.bss*)
     	*(.gnu.linkonce.b.*)
		*(COMMON*)
		. = ALIGN(4);
		PROVIDE( _ebss = .);
	} >RAM AT>FLASH

	PROVIDE( _end = _ebss);
	PROVIDE( end = . );

    .stack ORIGIN(RAM) + LENGTH(RAM) - __stack_size :
    {
        PROVIDE( _heap_end = . );
        . = ALIGN(4);
        PROVIDE(_susrstack = . );
        . = . + __stack_size;
        PROVIDE( _eusrstack = .);
    } >RAM

	/* 软复位不清。启动代码只清 _sbss 到 _ebss，扫不到这里。 */
	.share MEM_SHARE_ADDR (NOLOAD) :
	{
		KEEP(*(.share .share.*))
	} >SHARE

	ASSERT(ORIGIN(RAM) + LENGTH(RAM) == ORIGIN(SHARE), "RAM end is not the share area")
	ASSERT(SIZEOF(.share) <= LENGTH(SHARE), "region SHARE overflowed")

	/* 放在最后。空段不占字节，不会把 bin 从零等待区拉到 0x08010000。 */
	.nzwtext :
	{
		PROVIDE(_snzw = .);
		*(.nzwtext .nzwtext.*)
		*(.nzwrodata .nzwrodata.*)
		PROVIDE(_enzw = .);
	} >NZW AT>NZW

	ASSERT(_enzw >= _snzw, "nzw symbols missing")
	ASSERT((_enzw - _snzw) <= LENGTH(NZW), "region NZW overflowed")
}
