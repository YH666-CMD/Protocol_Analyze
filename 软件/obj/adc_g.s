	.file	"drv_adc.c"
	.option nopic
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.text.adc_irq_lock,"ax",@progbits
	.align	1
	.type	adc_irq_lock, @function
adc_irq_lock:
.LFB29:
	.file 1 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\fw_app\\03_Driver\\adc\\drv_adc.c"
	.loc 1 42 1
	.cfi_startproc
	.loc 1 43 5
	.loc 1 45 5
.LVL0:
.LBB17:
.LBB18:
	.file 2 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\Core/core_riscv.h"
	.loc 2 199 3
	.loc 2 199 32 is_stmt 0
	li	a5,-536813568
	lw	a0,0(a5)
.LVL1:
.LBE18:
.LBE17:
	.loc 1 46 5 is_stmt 1
.LBB20:
.LBB21:
	.loc 2 184 3
	.loc 2 184 39 is_stmt 0
	li	a4,134217728
	sw	a4,384(a5)
.LVL2:
.LBE21:
.LBE20:
	.loc 1 47 5 is_stmt 1
.LBB22:
.LBB19:
	.loc 2 199 10 is_stmt 0
	srli	a0,a0,27
.LVL3:
.LBE19:
.LBE22:
	.loc 1 48 1
	andi	a0,a0,1
	ret
	.cfi_endproc
.LFE29:
	.size	adc_irq_lock, .-adc_irq_lock
	.section	.text.adc_irq_unlock,"ax",@progbits
	.align	1
	.type	adc_irq_unlock, @function
adc_irq_unlock:
.LFB30:
	.loc 1 51 1 is_stmt 1
	.cfi_startproc
.LVL4:
	.loc 1 52 5
	.loc 1 52 8 is_stmt 0
	beqz	a0,.L2
	.loc 1 54 9 is_stmt 1
.LVL5:
.LBB23:
.LBB24:
	.loc 2 170 3
	.loc 2 170 39 is_stmt 0
	li	a5,-536813568
	li	a4,134217728
	sw	a4,256(a5)
.LVL6:
.L2:
.LBE24:
.LBE23:
	.loc 1 56 1
	ret
	.cfi_endproc
.LFE30:
	.size	adc_irq_unlock, .-adc_irq_unlock
	.section	.text.adc_q_clear,"ax",@progbits
	.align	1
	.type	adc_q_clear, @function
adc_q_clear:
.LFB41:
	.loc 1 284 1 is_stmt 1
	.cfi_startproc
	call	t0,__riscv_save_0
	.cfi_offset 1, -4
	.cfi_def_cfa_offset 16
	.loc 1 285 5
	.loc 1 287 5
	.loc 1 287 13 is_stmt 0
	call	adc_irq_lock
.LVL7:
	.loc 1 288 5 is_stmt 1
	.loc 1 288 11 is_stmt 0
	lui	a5,%hi(.LANCHOR0)
	sb	zero,%lo(.LANCHOR0)(a5)
	.loc 1 289 5 is_stmt 1
	.loc 1 289 11 is_stmt 0
	lui	a5,%hi(.LANCHOR1)
	sb	zero,%lo(.LANCHOR1)(a5)
	.loc 1 290 5 is_stmt 1
	.loc 1 290 11 is_stmt 0
	lui	a5,%hi(.LANCHOR2)
	sb	zero,%lo(.LANCHOR2)(a5)
	.loc 1 291 5 is_stmt 1
	call	adc_irq_unlock
.LVL8:
	.loc 1 292 1 is_stmt 0
	tail	__riscv_restore_0
	.cfi_restore 1
	.cfi_def_cfa_offset 0
	.cfi_endproc
.LFE41:
	.size	adc_q_clear, .-adc_q_clear
	.section	.text.adc_pins_idle,"ax",@progbits
	.align	1
	.type	adc_pins_idle, @function
adc_pins_idle:
.LFB31:
	.loc 1 59 1 is_stmt 1
	.cfi_startproc
	call	t0,__riscv_save_0
	.cfi_offset 1, -4
	.cfi_def_cfa_offset 16
	.loc 1 60 5
	.loc 1 62 5
	.loc 1 59 1 is_stmt 0
	addi	sp,sp,-16
	.cfi_def_cfa_offset 32
	.loc 1 62 5
	li	a1,1
	li	a0,4
	call	RCC_APB2PeriphClockCmd
.LVL9:
	.loc 1 63 5 is_stmt 1
	.loc 1 63 19 is_stmt 0
	li	a5,7
	.loc 1 66 5
	li	a0,1073811456
	.loc 1 63 19
	sh	a5,4(sp)
	.loc 1 64 5 is_stmt 1
	.loc 1 66 5 is_stmt 0
	addi	a1,sp,4
	.loc 1 64 21
	li	a5,3
	.loc 1 66 5
	addi	a0,a0,-2048
	.loc 1 64 21
	sw	a5,8(sp)
	.loc 1 65 5 is_stmt 1
	.loc 1 65 20 is_stmt 0
	sw	zero,12(sp)
	.loc 1 66 5 is_stmt 1
	call	GPIO_Init
.LVL10:
	.loc 1 67 1 is_stmt 0
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	tail	__riscv_restore_0
	.cfi_restore 1
	.cfi_def_cfa_offset -16
	.cfi_endproc
.LFE31:
	.size	adc_pins_idle, .-adc_pins_idle
	.section	.text.adc_stop,"ax",@progbits
	.align	1
	.type	adc_stop, @function
adc_stop:
.LFB40:
	.loc 1 264 1 is_stmt 1
	.cfi_startproc
	call	t0,__riscv_save_0
	.cfi_offset 1, -4
	.cfi_def_cfa_offset 16
	.loc 1 265 5
.LVL11:
.LBB25:
.LBB26:
	.loc 2 184 3
	.loc 2 184 39 is_stmt 0
	li	a5,-536813568
	li	a4,134217728
	sw	a4,384(a5)
.LVL12:
.LBE26:
.LBE25:
	.loc 1 266 5 is_stmt 1
.LBB27:
.LBB28:
	.loc 2 184 3
	.loc 2 184 39 is_stmt 0
	li	a4,4
	sw	a4,388(a5)
.LVL13:
.LBE28:
.LBE27:
	.loc 1 267 5 is_stmt 1
	.loc 1 267 13 is_stmt 0
	li	a5,1073876992
	lw	a5,20(a5)
	.loc 1 267 24
	andi	a5,a5,1
	.loc 1 267 8
	beqz	a5,.L12
	.loc 1 269 9 is_stmt 1
	.loc 1 269 29 is_stmt 0
	li	a5,1073872896
	lw	a4,8(a5)
	andi	a4,a4,-2
	sw	a4,8(a5)
	.loc 1 270 9 is_stmt 1
	.loc 1 270 29 is_stmt 0
	sw	zero,12(a5)
	.loc 1 271 9 is_stmt 1
	.loc 1 271 22 is_stmt 0
	li	a4,15
	sw	a4,4(a5)
.L12:
	.loc 1 273 5 is_stmt 1
	.loc 1 273 13 is_stmt 0
	li	a5,1073876992
	lw	a5,24(a5)
	.loc 1 273 25
	andi	a5,a5,512
	.loc 1 273 8
	beqz	a5,.L13
	.loc 1 275 9 is_stmt 1
	.loc 1 275 21 is_stmt 0
	li	a5,1073815552
	lw	a4,1032(a5)
	li	a3,-5242880
	addi	a3,a3,-260
	and	a4,a4,a3
	sw	a4,1032(a5)
.L13:
	.loc 1 277 5 is_stmt 1
	call	adc_pins_idle
.LVL14:
	.loc 1 278 5
	li	a1,0
	li	a0,512
	call	RCC_APB2PeriphClockCmd
.LVL15:
	.loc 1 279 5
	.loc 1 279 10 is_stmt 0
	lui	a5,%hi(.LANCHOR3)
	sb	zero,%lo(.LANCHOR3)(a5)
	.loc 1 280 5 is_stmt 1
	call	adc_q_clear
.LVL16:
	.loc 1 281 1 is_stmt 0
	tail	__riscv_restore_0
	.cfi_restore 1
	.cfi_def_cfa_offset 0
	.cfi_endproc
.LFE40:
	.size	adc_stop, .-adc_stop
	.section	.text.adc_q_push.part.0,"ax",@progbits
	.align	1
	.type	adc_q_push.part.0, @function
adc_q_push.part.0:
.LFB45:
	.loc 1 295 13 is_stmt 1
	.cfi_startproc
.LVL17:
	.loc 1 304 5
	.loc 1 304 15 is_stmt 0
	lui	a4,%hi(.LANCHOR2)
	addi	a5,a4,%lo(.LANCHOR2)
	lbu	a5,0(a5)
	.loc 1 304 8
	li	a3,9
	addi	a4,a4,%lo(.LANCHOR2)
	bgeu	a3,a5,.L23
	.loc 1 306 9 is_stmt 1
	.loc 1 306 34 is_stmt 0
	lui	a3,%hi(.LANCHOR1)
	addi	a3,a3,%lo(.LANCHOR1)
	lbu	a5,0(a3)
	.loc 1 306 17
	li	a2,10
	.loc 1 306 34
	addi	a5,a5,1
	.loc 1 306 17
	remu	a5,a5,a2
	.loc 1 306 15
	sb	a5,0(a3)
	.loc 1 307 9 is_stmt 1
	.loc 1 307 14 is_stmt 0
	lbu	a5,0(a4)
	.loc 1 308 12
	li	a3,-1
	.loc 1 307 14
	addi	a5,a5,-1
	andi	a5,a5,0xff
	sb	a5,0(a4)
	.loc 1 308 9 is_stmt 1
	.loc 1 308 22 is_stmt 0
	lui	a5,%hi(.LANCHOR4)
	addi	a5,a5,%lo(.LANCHOR4)
	lw	a2,0(a5)
	.loc 1 308 12
	beq	a2,a3,.L23
	.loc 1 310 13 is_stmt 1
	.loc 1 310 21 is_stmt 0
	lw	a3,0(a5)
	addi	a3,a3,1
	sw	a3,0(a5)
.L23:
	.loc 1 313 5 is_stmt 1
	.loc 1 313 10 is_stmt 0
	lui	a2,%hi(.LANCHOR0)
	addi	a5,a2,%lo(.LANCHOR0)
	lbu	a5,0(a5)
	li	a7,120
	.loc 1 316 22
	lui	a6,%hi(.LANCHOR5)
	.loc 1 313 10
	andi	a5,a5,0xff
	.loc 1 314 5 is_stmt 1
.LVL18:
	mul	a7,a5,a7
	.loc 1 313 10 is_stmt 0
	li	a3,0
	addi	a2,a2,%lo(.LANCHOR0)
	.loc 1 316 22
	addi	a6,a6,%lo(.LANCHOR5)
	.loc 1 314 5
	li	t1,120
.LVL19:
.L25:
	.loc 1 316 9 is_stmt 1
	.loc 1 316 27 is_stmt 0
	add	a1,a0,a3
	lhu	t3,0(a1)
	.loc 1 316 22
	add	a1,a3,a7
	add	a1,a6,a1
	sh	t3,0(a1)
	addi	a3,a3,2
	.loc 1 314 5
	bne	a3,t1,.L25
	.loc 1 318 5 is_stmt 1
	.loc 1 318 29 is_stmt 0
	addi	a5,a5,1
	.loc 1 318 13
	li	a3,10
	remu	a5,a5,a3
	.loc 1 318 11
	sb	a5,0(a2)
	.loc 1 319 5 is_stmt 1
	.loc 1 319 10 is_stmt 0
	lbu	a5,0(a4)
	addi	a5,a5,1
	andi	a5,a5,0xff
	sb	a5,0(a4)
	.loc 1 320 1
	ret
	.cfi_endproc
.LFE45:
	.size	adc_q_push.part.0, .-adc_q_push.part.0
	.section	.text.adc_take,"ax",@progbits
	.align	1
	.type	adc_take, @function
adc_take:
.LFB43:
	.loc 1 323 1 is_stmt 1
	.cfi_startproc
.LVL20:
	.loc 1 324 5
	.loc 1 325 5
	.loc 1 326 5
	.loc 1 327 5
	.loc 1 328 5
	.loc 1 329 5
	.loc 1 330 5
	.loc 1 332 5
	.loc 1 332 8 is_stmt 0
	beqz	a1,.L28
	.loc 1 334 9 is_stmt 1
	.loc 1 334 17 is_stmt 0
	sh	zero,0(a1)
.L28:
	.loc 1 336 5 is_stmt 1
	.loc 1 338 16 is_stmt 0
	li	a5,0
	.loc 1 336 8
	beqz	a0,.L55
	.loc 1 323 1
	call	t0,__riscv_save_2
	.cfi_offset 9, -12
	.cfi_offset 8, -8
	.cfi_offset 1, -4
	.cfi_def_cfa_offset 16
	addi	sp,sp,-128
	.cfi_def_cfa_offset 144
	mv	s1,a1
	mv	s0,a0
.LVL21:
.LBB33:
.LBB34:
	.loc 1 340 5 is_stmt 1
	.loc 1 340 13 is_stmt 0
	call	adc_irq_lock
.LVL22:
	.loc 1 341 5 is_stmt 1
	.loc 1 341 14 is_stmt 0
	lui	a5,%hi(.LANCHOR3)
	lbu	a5,%lo(.LANCHOR3)(a5)
	andi	a5,a5,0xff
	.loc 1 341 8
	beqz	a5,.L30
	.loc 1 341 29
	lui	a4,%hi(.LANCHOR2)
	addi	a5,a4,%lo(.LANCHOR2)
	lbu	a5,0(a5)
	addi	a4,a4,%lo(.LANCHOR2)
	andi	a5,a5,0xff
	.loc 1 341 20
	bnez	a5,.L31
.L30:
	.loc 1 343 9 is_stmt 1
	call	adc_irq_unlock
.LVL23:
	.loc 1 344 9
	.loc 1 344 16 is_stmt 0
	li	a5,0
.L29:
.LBE34:
.LBE33:
	.loc 1 368 1
	mv	a0,a5
	addi	sp,sp,128
	.cfi_remember_state
	.cfi_def_cfa_offset 0
	tail	__riscv_restore_2
	.cfi_restore 9
	.cfi_restore 8
	.cfi_restore 1
	.cfi_def_cfa_offset -16
.LVL24:
.L31:
	.cfi_restore_state
.LBB42:
.LBB41:
	.loc 1 346 5 is_stmt 1
	.loc 1 346 10 is_stmt 0
	lui	a2,%hi(.LANCHOR1)
	addi	a5,a2,%lo(.LANCHOR1)
	lbu	a5,0(a5)
	li	a7,120
	.loc 1 349 27
	lui	a6,%hi(.LANCHOR5)
	.loc 1 346 10
	andi	a5,a5,0xff
	.loc 1 347 5 is_stmt 1
.LVL25:
	mul	a7,a5,a7
	.loc 1 346 10 is_stmt 0
	li	a3,0
	addi	a2,a2,%lo(.LANCHOR1)
	.loc 1 349 27
	addi	a6,a6,%lo(.LANCHOR5)
	.loc 1 347 5
	li	t1,120
.LVL26:
.L33:
	.loc 1 349 9 is_stmt 1
	.loc 1 349 16 is_stmt 0
	addi	a1,sp,8
	add	t3,a1,a3
	.loc 1 349 27
	add	a1,a3,a7
	add	a1,a6,a1
	.loc 1 349 16
	lhu	a1,0(a1)
	addi	a3,a3,2
	sh	a1,0(t3)
	.loc 1 347 5
	bne	a3,t1,.L33
	.loc 1 351 5 is_stmt 1
	.loc 1 351 13 is_stmt 0
	li	a3,10
	.loc 1 351 29
	addi	a5,a5,1
	.loc 1 351 13
	remu	a5,a5,a3
	.loc 1 351 11
	sb	a5,0(a2)
	.loc 1 352 5 is_stmt 1
	.loc 1 352 10 is_stmt 0
	lbu	a5,0(a4)
	addi	a5,a5,-1
	andi	a5,a5,0xff
	sb	a5,0(a4)
	.loc 1 353 5 is_stmt 1
	call	adc_irq_unlock
.LVL27:
	.loc 1 355 5
	.loc 1 356 5
.LBB35:
.LBB36:
	.loc 1 195 24 is_stmt 0
	lui	a5,%hi(.LANCHOR6)
	lh	t3,%lo(.LANCHOR6)(a5)
	.loc 1 191 8
	li	a6,4096
	addi	a4,sp,8
	mv	a0,s0
	addi	t4,s0,40
	addi	t5,a6,-2
	.loc 1 193 16
	addi	t1,a6,-1
.LVL28:
.L34:
.LBE36:
.LBE35:
	.loc 1 346 10
	mv	a1,a0
	mv	a2,a4
	li	a3,3
.LVL29:
.L36:
	.loc 1 360 13 is_stmt 1
	.loc 1 360 46 is_stmt 0
	lhu	a5,0(a2)
.LVL30:
.LBB39:
.LBB37:
	.loc 1 185 5 is_stmt 1
	.loc 1 187 5
	.loc 1 187 8 is_stmt 0
	beqz	a5,.L35
	.loc 1 191 5 is_stmt 1
	.loc 1 191 8 is_stmt 0
	bgtu	a5,t5,.L39
	.loc 1 195 5 is_stmt 1
	.loc 1 195 7 is_stmt 0
	add	a7,a5,t3
.LVL31:
	.loc 1 196 5 is_stmt 1
	.loc 1 189 16 is_stmt 0
	li	a5,0
.LVL32:
	.loc 1 196 8
	bltz	a7,.L35
	.loc 1 200 5 is_stmt 1
	.loc 1 193 16 is_stmt 0
	mv	a5,t1
	.loc 1 200 8
	bge	a7,a6,.L35
	.loc 1 204 5 is_stmt 1
	.loc 1 204 12 is_stmt 0
	slli	a5,a7,16
	srli	a5,a5,16
.LVL33:
.L35:
	addi	a3,a3,-1
.LVL34:
.LBE37:
.LBE39:
	.loc 1 360 44
	sh	a5,0(a1)
	andi	a3,a3,0xff
	addi	a2,a2,2
	addi	a1,a1,40
	.loc 1 358 9
	bnez	a3,.L36
	addi	a0,a0,2
	addi	a4,a4,6
	.loc 1 356 5
	bne	a0,t4,.L34
	.loc 1 363 5 is_stmt 1
	.loc 1 367 12 is_stmt 0
	li	a5,1
	.loc 1 363 8
	beqz	s1,.L29
	.loc 1 365 9 is_stmt 1
	.loc 1 365 17 is_stmt 0
	li	a5,20
	sh	a5,0(s1)
	.loc 1 367 12
	li	a5,1
	j	.L29
.LVL35:
.L39:
.LBB40:
.LBB38:
	.loc 1 193 16
	mv	a5,t1
.LVL36:
	j	.L35
.LVL37:
.L55:
	.cfi_def_cfa_offset 0
	.cfi_restore 1
	.cfi_restore 8
	.cfi_restore 9
.LBE38:
.LBE40:
.LBE41:
.LBE42:
	.loc 1 368 1
	mv	a0,a5
.LVL38:
	ret
	.cfi_endproc
.LFE43:
	.size	adc_take, .-adc_take
	.section	.text.adc_start,"ax",@progbits
	.align	1
	.type	adc_start, @function
adc_start:
.LFB39:
	.loc 1 223 1 is_stmt 1
	.cfi_startproc
	call	t0,__riscv_save_3
	.cfi_offset 18, -16
	.cfi_offset 9, -12
	.cfi_offset 8, -8
	.cfi_offset 1, -4
	.cfi_def_cfa_offset 16
	.loc 1 224 5
	.loc 1 226 5
	.loc 1 226 14 is_stmt 0
	lui	s0,%hi(.LANCHOR3)
	addi	a5,s0,%lo(.LANCHOR3)
	lbu	a5,0(a5)
	.loc 1 223 1
	addi	sp,sp,-32
	.cfi_def_cfa_offset 48
	.loc 1 228 16
	li	a0,0
	.loc 1 226 14
	andi	a5,a5,0xff
	.loc 1 226 8
	bnez	a5,.L58
.LBB62:
.LBB63:
	.loc 1 231 17
	lui	a5,%hi(pp_sw_driver)
	.loc 1 231 5
	lw	a5,%lo(pp_sw_driver)(a5)
	lui	s1,%hi(.LANCHOR7)
	lui	a2,%hi(adc_irq_unlock)
	lui	a1,%hi(adc_irq_lock)
	addi	a2,a2,%lo(adc_irq_unlock)
	addi	a1,a1,%lo(adc_irq_lock)
	addi	a0,s1,%lo(.LANCHOR7)
	jalr	a5
.LVL39:
	.loc 1 232 5
	call	adc_pins_idle
.LVL40:
	.loc 1 233 22
	li	a4,1073876992
	lw	a5,4(a4)
	.loc 1 233 45
	li	a3,49152
	.loc 1 234 5
	li	a1,1
	.loc 1 233 45
	or	a5,a5,a3
	.loc 1 233 16
	sw	a5,4(a4)
	.loc 1 234 5
	li	a0,512
	call	RCC_APB2PeriphClockCmd
.LVL41:
	.loc 1 235 5
	li	a1,1
	li	a0,1
	call	RCC_AHBPeriphClockCmd
.LVL42:
	.loc 1 237 17
	li	a5,1073815552
	lw	a4,1032(a5)
	addi	a3,sp,12
	addi	s0,s0,%lo(.LANCHOR3)
	.loc 1 231 5 is_stmt 1
	.loc 1 232 5
	.loc 1 233 5
	.loc 1 234 5
	.loc 1 235 5
	.loc 1 237 5
	.loc 1 237 17 is_stmt 0
	andi	a4,a4,-2
	sw	a4,1032(a5)
	.loc 1 238 5 is_stmt 1
	.loc 1 238 17 is_stmt 0
	li	a4,256
	sw	a4,1028(a5)
	.loc 1 239 5 is_stmt 1
	.loc 1 239 17 is_stmt 0
	li	a4,917504
	addi	a4,a4,2
	sw	a4,1032(a5)
	.loc 1 240 5 is_stmt 1
	.loc 1 240 17 is_stmt 0
	li	a4,2097152
	sw	a4,1068(a5)
	.loc 1 241 5 is_stmt 1
	.loc 1 242 17 is_stmt 0
	li	a4,4096
	.loc 1 241 17
	sw	zero,1072(a5)
	.loc 1 242 5 is_stmt 1
	.loc 1 242 17 is_stmt 0
	addi	a4,a4,-2016
	sw	a4,1076(a5)
	.loc 1 243 5 is_stmt 1
.LVL43:
	.loc 1 244 5
	.loc 1 244 19 is_stmt 0
	li	a4,511
	sw	a4,1040(a5)
	.loc 1 245 5 is_stmt 1
	.loc 1 245 19 is_stmt 0
	sw	zero,1036(a5)
	.loc 1 246 5 is_stmt 1
	.loc 1 246 17 is_stmt 0
	lw	a4,1032(a5)
	addi	a7,sp,32
	mv	a1,a3
	ori	a4,a4,1
	sw	a4,1032(a5)
	.loc 1 247 5 is_stmt 1
.LBB64:
.LBB65:
	.loc 1 168 5
	.loc 1 169 5
	.loc 1 171 5
.LVL44:
.LBB66:
.LBB67:
	.loc 1 85 17 is_stmt 0
	li	a0,1073815552
.LBE67:
.LBE66:
.LBE65:
.LBE64:
	.loc 1 246 17
	mv	a4,a3
.LVL45:
.L76:
.LBB94:
.LBB90:
	.loc 1 173 9 is_stmt 1
.LBB77:
.LBB74:
	.loc 1 85 5
	.loc 1 85 17 is_stmt 0
	lw	a5,1032(a0)
	ori	a5,a5,8
	sw	a5,1032(a0)
	.loc 1 86 5 is_stmt 1
.LVL46:
	.loc 1 85 17 is_stmt 0
	li	a5,2000
.LVL47:
.L61:
.LBB68:
.LBB69:
	.loc 1 75 9 is_stmt 1
	.loc 1 75 18 is_stmt 0
	addi	a2,a0,1024
	lw	a6,8(a2)
	.loc 1 75 26
	andi	a6,a6,8
	.loc 1 75 12
	beqz	a6,.L60
.LVL48:
	addi	a5,a5,-1
.LVL49:
	slli	a5,a5,16
	srli	a5,a5,16
	.loc 1 73 5
	bnez	a5,.L61
.LVL50:
.L62:
.LBE69:
.LBE68:
.LBE74:
.LBE77:
.LBE90:
.LBE94:
	.loc 1 249 9 is_stmt 1
	call	adc_stop
.LVL51:
	.loc 1 250 9
	.loc 1 250 16 is_stmt 0
	li	a0,5
	j	.L58
.LVL52:
.L71:
.LBB95:
.LBB91:
.LBB78:
.LBB79:
	.loc 1 134 9 is_stmt 1
	.loc 1 134 11 is_stmt 0
	add	a2,a4,a2
.LVL53:
	.loc 1 135 10
	addi	a5,a5,1
.LVL54:
	.loc 1 134 11
	slli	a2,a2,16
	.loc 1 135 10
	andi	a5,a5,0xff
	.loc 1 134 11
	srli	a2,a2,16
.LVL55:
	.loc 1 135 9 is_stmt 1
	addi	a3,a3,2
	.loc 1 132 11 is_stmt 0
	bne	a5,a6,.L69
.L70:
	.loc 1 139 9 is_stmt 1
	.loc 1 139 39 is_stmt 0
	rem	a4,a2,a5
	.loc 1 139 44
	slli	a4,a4,1
	.loc 1 139 27
	div	a3,a2,a5
	.loc 1 139 50
	divu	a4,a4,a5
	.loc 1 139 11
	add	a4,a4,a3
	slli	a3,a4,16
	srli	a3,a3,16
.LVL56:
	j	.L72
.LVL57:
.L75:
	.loc 1 151 13 is_stmt 1
	.loc 1 151 35 is_stmt 0
	slli	a0,a5,1
	add	a0,a1,a0
	.loc 1 151 15
	lhu	a0,4(a0)
	addi	a5,a5,1
.LVL58:
	add	a4,a4,a0
.LVL59:
	slli	a4,a4,16
	srli	a4,a4,16
.LVL60:
	.loc 1 152 13 is_stmt 1
	j	.L74
.LVL61:
.L80:
	.loc 1 158 11 is_stmt 0
	li	a5,4096
.LVL62:
	addi	a5,a5,-2048
	j	.L73
.LVL63:
.L63:
.LBE79:
.LBE78:
.LBB82:
.LBB75:
	.loc 1 95 5 is_stmt 1
	.loc 1 95 26 is_stmt 0
	lw	a5,76(a6)
	addi	a4,a4,2
	.loc 1 95 12
	sh	a5,-2(a4)
	.loc 1 96 5 is_stmt 1
.LBE75:
.LBE82:
	.loc 1 171 5 is_stmt 0
	bne	a7,a4,.L76
	addi	a2,sp,14
.LBB83:
.LBB84:
	.loc 1 104 12
	li	a4,1
	.loc 1 104 5
	li	t1,10
.L68:
.LVL64:
.LBB85:
	.loc 1 106 9 is_stmt 1
	.loc 1 108 9
	.loc 1 108 11 is_stmt 0
	lhu	a6,0(a2)
.LVL65:
	.loc 1 109 9 is_stmt 1
	.loc 1 110 9
	.loc 1 108 11 is_stmt 0
	mv	a0,a2
	mv	a5,a4
.LVL66:
.L65:
	.loc 1 110 29
	lhu	a7,-2(a0)
	.loc 1 110 23
	bgeu	a6,a7,.L66
	.loc 1 112 13 is_stmt 1
	.loc 1 113 14 is_stmt 0
	addi	a5,a5,-1
.LVL67:
	.loc 1 112 20
	sh	a7,0(a0)
	.loc 1 113 13 is_stmt 1
	.loc 1 113 14 is_stmt 0
	andi	a5,a5,0xff
.LVL68:
	addi	a0,a0,-2
	.loc 1 110 15
	bnez	a5,.L65
.L66:
	.loc 1 115 9 is_stmt 1
	.loc 1 115 16 is_stmt 0
	slli	a5,a5,1
.LVL69:
	addi	a0,sp,32
	add	a5,a0,a5
.LBE85:
	.loc 1 104 26
	addi	a4,a4,1
.LVL70:
.LBB86:
	.loc 1 115 16
	sh	a6,-20(a5)
.LBE86:
	.loc 1 104 26
	andi	a4,a4,0xff
.LVL71:
	addi	a2,a2,2
	.loc 1 104 5
	bne	a4,t1,.L68
.LBE84:
.LBE83:
.LBB87:
.LBB80:
	.loc 1 131 7
	li	a5,0
	.loc 1 129 7
	li	a2,0
	.loc 1 132 19
	li	a0,1536
	.loc 1 132 11
	li	a6,6
.LVL72:
.L69:
	.loc 1 132 25
	lhu	a4,4(a3)
	.loc 1 132 19
	bleu	a4,a0,.L71
	.loc 1 137 5 is_stmt 1
	.loc 1 143 11 is_stmt 0
	li	a3,1024
	.loc 1 137 8
	bnez	a5,.L70
.LVL73:
.L72:
	.loc 1 146 5 is_stmt 1
	.loc 1 146 7 is_stmt 0
	li	a4,6
	sub	a4,a4,a5
	andi	a2,a4,0xff
.LVL74:
	.loc 1 147 5 is_stmt 1
	.loc 1 147 8 is_stmt 0
	beqz	a2,.L80
	.loc 1 130 7
	li	a4,0
	.loc 1 149 15
	li	a6,5
.LVL75:
.L74:
	andi	a0,a5,0xff
	bleu	a0,a6,.L75
	.loc 1 154 9 is_stmt 1
	.loc 1 154 39 is_stmt 0
	rem	a5,a4,a2
.LVL76:
	.loc 1 154 44
	slli	a5,a5,1
	.loc 1 154 50
	divu	a5,a5,a2
	.loc 1 154 27
	div	a4,a4,a2
.LVL77:
	.loc 1 154 11
	add	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
.LVL78:
.L73:
	.loc 1 161 5 is_stmt 1
	.loc 1 162 5
	.loc 1 163 5
	.loc 1 162 24 is_stmt 0
	li	s2,4096
	.loc 1 161 24
	li	a4,1024
	sub	a4,a4,a3
	.loc 1 162 24
	addi	a3,s2,-2048
.LVL79:
	sub	a5,a3,a5
.LVL80:
	.loc 1 163 26
	slli	a4,a4,16
	slli	a5,a5,16
	srai	a4,a4,16
	srai	a5,a5,16
	.loc 1 163 31
	li	a3,2
	.loc 1 163 26
	add	a5,a4,a5
.LVL81:
	.loc 1 163 31
	div	a4,a5,a3
.LBE80:
.LBE87:
.LBE91:
.LBE95:
.LBB96:
.LBB97:
	.loc 1 211 28
	addi	s1,s1,%lo(.LANCHOR7)
	.loc 1 213 25
	addi	s2,s2,1446
.LBE97:
.LBE96:
.LBB100:
.LBB92:
.LBB88:
.LBB81:
	.loc 1 163 47
	rem	a5,a5,a3
	.loc 1 163 36
	add	a5,a4,a5
.LBE81:
.LBE88:
	.loc 1 179 11
	lui	a4,%hi(.LANCHOR6)
	sh	a5,%lo(.LANCHOR6)(a4)
	.loc 1 180 5 is_stmt 1
.LBE92:
.LBE100:
	.loc 1 253 5
	call	adc_q_clear
.LVL82:
	.loc 1 254 5
	.loc 1 254 14 is_stmt 0
	lui	a5,%hi(.LANCHOR4)
	sw	zero,%lo(.LANCHOR4)(a5)
	.loc 1 255 5 is_stmt 1
	.loc 1 255 10 is_stmt 0
	li	a5,1
	sb	a5,0(s0)
	.loc 1 256 5 is_stmt 1
.LBB101:
.LBB98:
	.loc 1 209 5
	.loc 1 209 25 is_stmt 0
	li	a4,1073872896
	lw	a5,8(a4)
	.loc 1 216 18
	li	a3,15
.LBE98:
.LBE101:
	.loc 1 260 12
	li	a0,0
.LBB102:
.LBB99:
	.loc 1 209 25
	andi	a5,a5,-2
	sw	a5,8(a4)
	.loc 1 210 5 is_stmt 1
	.loc 1 210 26 is_stmt 0
	li	a5,1073815552
	addi	a2,a5,1100
	sw	a2,16(a4)
	.loc 1 211 5 is_stmt 1
	.loc 1 211 26 is_stmt 0
	sw	s1,20(a4)
	.loc 1 212 5 is_stmt 1
	.loc 1 212 25 is_stmt 0
	li	a2,120
	sw	a2,12(a4)
	.loc 1 213 5 is_stmt 1
	.loc 1 213 25 is_stmt 0
	sw	s2,8(a4)
	.loc 1 216 5 is_stmt 1
	.loc 1 216 18 is_stmt 0
	sw	a3,4(a4)
	.loc 1 217 5 is_stmt 1
	.loc 1 217 25 is_stmt 0
	lw	a3,8(a4)
	ori	a3,a3,1
	sw	a3,8(a4)
.LBE99:
.LBE102:
	.loc 1 257 5 is_stmt 1
	.loc 1 257 17 is_stmt 0
	lw	a4,1032(a5)
.LBB103:
.LBB104:
	.loc 2 170 39
	li	a3,134217728
.LBE104:
.LBE103:
	.loc 1 257 17
	ori	a4,a4,256
	sw	a4,1032(a5)
	.loc 1 258 5 is_stmt 1
.LVL83:
.LBB106:
.LBB105:
	.loc 2 170 3
	.loc 2 170 39 is_stmt 0
	li	a4,-536813568
	sw	a3,256(a4)
.LVL84:
.LBE105:
.LBE106:
	.loc 1 259 5 is_stmt 1
	.loc 1 259 17 is_stmt 0
	lw	a4,1032(a5)
	li	a3,5242880
	or	a4,a4,a3
	sw	a4,1032(a5)
	.loc 1 260 5 is_stmt 1
.LVL85:
.L58:
.LBE63:
.LBE62:
	.loc 1 261 1 is_stmt 0
	addi	sp,sp,32
	.cfi_remember_state
	.cfi_def_cfa_offset 0
	tail	__riscv_restore_3
	.cfi_restore 18
	.cfi_restore 9
	.cfi_restore 8
	.cfi_restore 1
	.cfi_def_cfa_offset -16
.LVL86:
.L60:
	.cfi_restore_state
.LBB109:
.LBB108:
.LBB107:
.LBB93:
.LBB89:
.LBB76:
	.loc 1 90 5 is_stmt 1
	.loc 1 90 17 is_stmt 0
	lw	a5,8(a2)
	ori	a5,a5,4
	sw	a5,8(a2)
	.loc 1 91 5 is_stmt 1
.LVL87:
.LBB70:
.LBB71:
	.loc 1 71 5
	.loc 1 73 5
.LBE71:
.LBE70:
	.loc 1 90 17 is_stmt 0
	li	a5,2000
.LVL88:
.L64:
.LBB73:
.LBB72:
	.loc 1 75 9 is_stmt 1
	.loc 1 75 18 is_stmt 0
	addi	a6,a0,1024
	lw	a2,8(a6)
	.loc 1 75 26
	andi	a2,a2,4
	.loc 1 75 12
	beqz	a2,.L63
.LVL89:
	addi	a5,a5,-1
.LVL90:
	slli	a5,a5,16
	srli	a5,a5,16
	.loc 1 73 5
	bnez	a5,.L64
	j	.L62
.LBE72:
.LBE73:
.LBE76:
.LBE89:
.LBE93:
.LBE107:
.LBE108:
.LBE109:
	.cfi_endproc
.LFE39:
	.size	adc_start, .-adc_start
	.section	.text.DMA1_Channel1_IRQHandler,"ax",@progbits
	.align	1
	.globl	DMA1_Channel1_IRQHandler
	.type	DMA1_Channel1_IRQHandler, @function
DMA1_Channel1_IRQHandler:
.LFB44:
	.loc 1 373 1 is_stmt 1
	.cfi_startproc
	.loc 1 374 5
	.loc 1 376 5
	.loc 1 373 1 is_stmt 0
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	.loc 1 376 11
	li	a5,1073872896
	.loc 1 373 1
	sw	s0,12(sp)
	.cfi_offset 8, -4
	.loc 1 376 11
	lw	s0,0(a5)
.LVL91:
	.loc 1 377 5 is_stmt 1
	.loc 1 377 16 is_stmt 0
	andi	a4,s0,4
	.loc 1 377 8
	beqz	a4,.L93
	.loc 1 379 9 is_stmt 1
	.loc 1 379 22 is_stmt 0
	li	a4,4
	sw	a4,4(a5)
	.loc 1 380 9 is_stmt 1
.LVL92:
.LBB114:
.LBB115:
	.loc 1 297 5
	.loc 1 298 5
	.loc 1 300 5
	.loc 1 300 14 is_stmt 0
	lui	a5,%hi(.LANCHOR3)
	lbu	a5,%lo(.LANCHOR3)(a5)
	andi	a5,a5,0xff
	.loc 1 300 8
	beqz	a5,.L93
	lui	a0,%hi(.LANCHOR7)
	addi	a0,a0,%lo(.LANCHOR7)
	call	adc_q_push.part.0
.LVL93:
.L93:
.LBE115:
.LBE114:
	.loc 1 382 5 is_stmt 1
	.loc 1 382 16 is_stmt 0
	andi	s0,s0,2
.LVL94:
	.loc 1 382 8
	beqz	s0,.L91
	.loc 1 384 9 is_stmt 1
	.loc 1 384 22 is_stmt 0
	li	a5,1073872896
	li	a4,2
	sw	a4,4(a5)
	.loc 1 385 9 is_stmt 1
.LVL95:
.LBB116:
.LBB117:
	.loc 1 297 5
	.loc 1 298 5
	.loc 1 300 5
	.loc 1 300 14 is_stmt 0
	lui	a5,%hi(.LANCHOR3)
	lbu	a5,%lo(.LANCHOR3)(a5)
	andi	a5,a5,0xff
	.loc 1 300 8
	beqz	a5,.L91
	lui	a0,%hi(.LANCHOR7+120)
	addi	a0,a0,%lo(.LANCHOR7+120)
	call	adc_q_push.part.0
.LVL96:
.L91:
.LBE117:
.LBE116:
	.loc 1 387 1
	lw	s0,12(sp)
	.cfi_restore 8
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	mret
	.cfi_endproc
.LFE44:
	.size	DMA1_Channel1_IRQHandler, .-DMA1_Channel1_IRQHandler
	.globl	drv_adc
	.section	.bss.s_pp,"aw",@nobits
	.align	2
	.set	.LANCHOR7,. + 0
	.type	s_pp, @object
	.size	s_pp, 256
s_pp:
	.zero	256
	.section	.bss.s_q,"aw",@nobits
	.align	2
	.set	.LANCHOR5,. + 0
	.type	s_q, @object
	.size	s_q, 1200
s_q:
	.zero	1200
	.section	.rodata.drv_adc,"a"
	.align	2
	.type	drv_adc, @object
	.size	drv_adc, 12
drv_adc:
	.word	adc_start
	.word	adc_stop
	.word	adc_take
	.section	.sbss.s_cal,"aw",@nobits
	.align	1
	.set	.LANCHOR6,. + 0
	.type	s_cal, @object
	.size	s_cal, 2
s_cal:
	.zero	2
	.section	.sbss.s_on,"aw",@nobits
	.set	.LANCHOR3,. + 0
	.type	s_on, @object
	.size	s_on, 1
s_on:
	.zero	1
	.section	.sbss.s_q_drop,"aw",@nobits
	.align	2
	.set	.LANCHOR4,. + 0
	.type	s_q_drop, @object
	.size	s_q_drop, 4
s_q_drop:
	.zero	4
	.section	.sbss.s_q_n,"aw",@nobits
	.set	.LANCHOR2,. + 0
	.type	s_q_n, @object
	.size	s_q_n, 1
s_q_n:
	.zero	1
	.section	.sbss.s_q_r,"aw",@nobits
	.set	.LANCHOR1,. + 0
	.type	s_q_r, @object
	.size	s_q_r, 1
s_q_r:
	.zero	1
	.section	.sbss.s_q_w,"aw",@nobits
	.set	.LANCHOR0,. + 0
	.type	s_q_w, @object
	.size	s_q_w, 1
s_q_w:
	.zero	1
	.text
.Letext0:
	.file 3 "e:\\wch_ide_yh\\mounriver_studio\\toolchain\\risc-v embedded gcc\\riscv-none-embed\\include\\machine\\_default_types.h"
	.file 4 "e:\\wch_ide_yh\\mounriver_studio\\toolchain\\risc-v embedded gcc\\riscv-none-embed\\include\\sys\\_stdint.h"
	.file 5 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\fw_app\\00_System\\System/platform.h"
	.file 6 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\fw_app\\04_Components\\pingpong_buf/pingpong_buf.h"
	.file 7 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\fw_app\\03_Driver\\adc\\drv_adc.h"
	.file 8 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\Peripheral\\inc/ch32v20x.h"
	.file 9 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\User/system_ch32v20x.h"
	.file 10 "e:\\wch_ide_yh\\mounriver_studio\\toolchain\\risc-v embedded gcc\\riscv-none-embed\\include\\sys\\lock.h"
	.file 11 "e:\\wch_ide_yh\\mounriver_studio\\toolchain\\risc-v embedded gcc\\riscv-none-embed\\include\\sys\\_types.h"
	.file 12 "e:\\wch_ide_yh\\mounriver_studio\\toolchain\\risc-v embedded gcc\\lib\\gcc\\riscv-none-embed\\8.2.0\\include\\stddef.h"
	.file 13 "e:\\wch_ide_yh\\mounriver_studio\\toolchain\\risc-v embedded gcc\\riscv-none-embed\\include\\sys\\reent.h"
	.file 14 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\Peripheral\\inc/ch32v20x_gpio.h"
	.file 15 "D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\Peripheral\\inc/ch32v20x_rcc.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.4byte	0x1d0f
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.byte	0x1
	.4byte	.LASF356
	.byte	0xc
	.4byte	.LASF357
	.4byte	.LASF358
	.4byte	.Ldebug_ranges0+0x148
	.4byte	0
	.4byte	.Ldebug_line0
	.byte	0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF0
	.byte	0x3
	.4byte	.LASF2
	.byte	0x3
	.byte	0x2b
	.byte	0x18
	.4byte	0x38
	.byte	0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.byte	0x3
	.4byte	.LASF3
	.byte	0x3
	.byte	0x37
	.byte	0x18
	.4byte	0x4b
	.byte	0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF4
	.byte	0x3
	.4byte	.LASF5
	.byte	0x3
	.byte	0x39
	.byte	0x19
	.4byte	0x5e
	.byte	0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF6
	.byte	0x3
	.4byte	.LASF7
	.byte	0x3
	.byte	0x4d
	.byte	0x18
	.4byte	0x71
	.byte	0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF8
	.byte	0x3
	.4byte	.LASF9
	.byte	0x3
	.byte	0x4f
	.byte	0x19
	.4byte	0x84
	.byte	0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF10
	.byte	0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF11
	.byte	0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF12
	.byte	0x4
	.byte	0x4
	.byte	0x5
	.string	"int"
	.byte	0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF13
	.byte	0x3
	.4byte	.LASF14
	.byte	0x4
	.byte	0x18
	.byte	0x13
	.4byte	0x2c
	.byte	0x5
	.4byte	0xa7
	.byte	0x3
	.4byte	.LASF15
	.byte	0x4
	.byte	0x20
	.byte	0x13
	.4byte	0x3f
	.byte	0x3
	.4byte	.LASF16
	.byte	0x4
	.byte	0x24
	.byte	0x14
	.4byte	0x52
	.byte	0x6
	.4byte	0xc4
	.byte	0x3
	.4byte	.LASF17
	.byte	0x4
	.byte	0x2c
	.byte	0x13
	.4byte	0x65
	.byte	0x3
	.4byte	.LASF18
	.byte	0x4
	.byte	0x30
	.byte	0x14
	.4byte	0x78
	.byte	0x5
	.4byte	0xe1
	.byte	0x6
	.4byte	0xed
	.byte	0x7
	.byte	0x7
	.byte	0x4
	.4byte	0xa0
	.byte	0x5
	.byte	0x14
	.byte	0x1
	.4byte	0x12a
	.byte	0x8
	.4byte	.LASF19
	.byte	0
	.byte	0x8
	.4byte	.LASF20
	.byte	0x1
	.byte	0x8
	.4byte	.LASF21
	.byte	0x2
	.byte	0x8
	.4byte	.LASF22
	.byte	0x3
	.byte	0x8
	.4byte	.LASF23
	.byte	0x4
	.byte	0x8
	.4byte	.LASF24
	.byte	0x5
	.byte	0
	.byte	0x3
	.4byte	.LASF25
	.byte	0x5
	.byte	0x1b
	.byte	0x3
	.4byte	0xf7
	.byte	0x3
	.4byte	.LASF26
	.byte	0x5
	.byte	0x3b
	.byte	0x13
	.4byte	0x142
	.byte	0x9
	.byte	0x4
	.4byte	0x148
	.byte	0xa
	.4byte	0xd5
	.4byte	0x15c
	.byte	0xb
	.4byte	0xe1
	.byte	0xb
	.4byte	0xe1
	.byte	0
	.byte	0xc
	.byte	0xc
	.byte	0x5
	.byte	0x3d
	.byte	0x9
	.4byte	0x18c
	.byte	0xd
	.4byte	.LASF27
	.byte	0x5
	.byte	0x3f
	.byte	0x12
	.4byte	0x18c
	.byte	0
	.byte	0xe
	.string	"fn"
	.byte	0x5
	.byte	0x40
	.byte	0x12
	.4byte	0x136
	.byte	0x4
	.byte	0xd
	.4byte	.LASF28
	.byte	0x5
	.byte	0x41
	.byte	0x12
	.4byte	0xa7
	.byte	0x8
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x199
	.byte	0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF29
	.byte	0x6
	.4byte	0x192
	.byte	0x3
	.4byte	.LASF30
	.byte	0x5
	.byte	0x42
	.byte	0x3
	.4byte	0x15c
	.byte	0x6
	.4byte	0x19e
	.byte	0xc
	.byte	0x10
	.byte	0x5
	.byte	0x44
	.byte	0x9
	.4byte	0x1ed
	.byte	0xd
	.4byte	.LASF27
	.byte	0x5
	.byte	0x46
	.byte	0x11
	.4byte	0x18c
	.byte	0
	.byte	0xd
	.4byte	.LASF31
	.byte	0x5
	.byte	0x47
	.byte	0x11
	.4byte	0xe1
	.byte	0x4
	.byte	0xd
	.4byte	.LASF32
	.byte	0x5
	.byte	0x48
	.byte	0x11
	.4byte	0xe1
	.byte	0x8
	.byte	0xd
	.4byte	.LASF33
	.byte	0x5
	.byte	0x49
	.byte	0x11
	.4byte	0xd5
	.byte	0xc
	.byte	0
	.byte	0x3
	.4byte	.LASF34
	.byte	0x5
	.byte	0x4a
	.byte	0x3
	.4byte	0x1af
	.byte	0x5
	.4byte	0x1ed
	.byte	0xf
	.4byte	.LASF35
	.byte	0x5
	.byte	0x4d
	.byte	0x23
	.4byte	0x1f9
	.byte	0xf
	.4byte	.LASF36
	.byte	0x5
	.byte	0x50
	.byte	0x1a
	.4byte	0xed
	.byte	0xf
	.4byte	.LASF37
	.byte	0x5
	.byte	0x5b
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF38
	.byte	0x5
	.byte	0x5c
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF39
	.byte	0x5
	.byte	0x5d
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF40
	.byte	0x5
	.byte	0x5e
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF41
	.byte	0x5
	.byte	0x5f
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF42
	.byte	0x5
	.byte	0x60
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF43
	.byte	0x5
	.byte	0x61
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF44
	.byte	0x5
	.byte	0x62
	.byte	0x1a
	.4byte	0x1aa
	.byte	0xf
	.4byte	.LASF45
	.byte	0x5
	.byte	0x63
	.byte	0x1a
	.4byte	0x1aa
	.byte	0x3
	.4byte	.LASF46
	.byte	0x6
	.byte	0x18
	.byte	0x12
	.4byte	0xc4
	.byte	0x3
	.4byte	.LASF47
	.byte	0x6
	.byte	0x1b
	.byte	0x14
	.4byte	0x29a
	.byte	0x9
	.byte	0x4
	.4byte	0x2a0
	.byte	0x10
	.4byte	0xe1
	.byte	0x3
	.4byte	.LASF48
	.byte	0x6
	.byte	0x1c
	.byte	0x14
	.4byte	0x2b1
	.byte	0x9
	.byte	0x4
	.4byte	0x2b7
	.byte	0x11
	.4byte	0x2c2
	.byte	0xb
	.4byte	0xe1
	.byte	0
	.byte	0x12
	.2byte	0x100
	.byte	0x6
	.byte	0x2d
	.byte	0x9
	.4byte	0x30e
	.byte	0xe
	.string	"buf"
	.byte	0x6
	.byte	0x2f
	.byte	0x17
	.4byte	0x30e
	.byte	0
	.byte	0xd
	.4byte	.LASF49
	.byte	0x6
	.byte	0x30
	.byte	0x17
	.4byte	0xb3
	.byte	0xf0
	.byte	0xd
	.4byte	.LASF50
	.byte	0x6
	.byte	0x31
	.byte	0x17
	.4byte	0xed
	.byte	0xf4
	.byte	0xd
	.4byte	.LASF51
	.byte	0x6
	.byte	0x32
	.byte	0x17
	.4byte	0x28e
	.byte	0xf8
	.byte	0xd
	.4byte	.LASF52
	.byte	0x6
	.byte	0x33
	.byte	0x17
	.4byte	0x2a5
	.byte	0xfc
	.byte	0
	.byte	0x13
	.4byte	0x282
	.4byte	0x31e
	.byte	0x14
	.4byte	0xa0
	.byte	0x77
	.byte	0
	.byte	0x3
	.4byte	.LASF53
	.byte	0x6
	.byte	0x34
	.byte	0x3
	.4byte	0x2c2
	.byte	0xc
	.byte	0x10
	.byte	0x6
	.byte	0x3a
	.byte	0x9
	.4byte	0x368
	.byte	0xd
	.4byte	.LASF54
	.byte	0x6
	.byte	0x3c
	.byte	0xc
	.4byte	0x383
	.byte	0
	.byte	0xd
	.4byte	.LASF55
	.byte	0x6
	.byte	0x3d
	.byte	0xc
	.4byte	0x394
	.byte	0x4
	.byte	0xd
	.4byte	.LASF56
	.byte	0x6
	.byte	0x3e
	.byte	0xc
	.4byte	0x394
	.byte	0x8
	.byte	0xd
	.4byte	.LASF57
	.byte	0x6
	.byte	0x43
	.byte	0xc
	.4byte	0x3cb
	.byte	0xc
	.byte	0
	.byte	0x11
	.4byte	0x37d
	.byte	0xb
	.4byte	0x37d
	.byte	0xb
	.4byte	0x28e
	.byte	0xb
	.4byte	0x2a5
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x31e
	.byte	0x9
	.byte	0x4
	.4byte	0x368
	.byte	0x11
	.4byte	0x394
	.byte	0xb
	.4byte	0x37d
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x389
	.byte	0xa
	.4byte	0x3b8
	.4byte	0x3b8
	.byte	0xb
	.4byte	0x37d
	.byte	0xb
	.4byte	0xa7
	.byte	0xb
	.4byte	0x3bf
	.byte	0xb
	.4byte	0x3c5
	.byte	0
	.byte	0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF58
	.byte	0x9
	.byte	0x4
	.4byte	0x282
	.byte	0x9
	.byte	0x4
	.4byte	0xc4
	.byte	0x9
	.byte	0x4
	.4byte	0x39a
	.byte	0x3
	.4byte	.LASF59
	.byte	0x6
	.byte	0x44
	.byte	0x3
	.4byte	0x32a
	.byte	0x6
	.4byte	0x3d1
	.byte	0xf
	.4byte	.LASF60
	.byte	0x6
	.byte	0x47
	.byte	0x24
	.4byte	0x3dd
	.byte	0xc
	.byte	0xc
	.byte	0x7
	.byte	0x17
	.byte	0x9
	.4byte	0x41f
	.byte	0xd
	.4byte	.LASF61
	.byte	0x7
	.byte	0x1d
	.byte	0x10
	.4byte	0x424
	.byte	0
	.byte	0xd
	.4byte	.LASF62
	.byte	0x7
	.byte	0x21
	.byte	0xc
	.4byte	0x42b
	.byte	0x4
	.byte	0xd
	.4byte	.LASF63
	.byte	0x7
	.byte	0x27
	.byte	0xc
	.4byte	0x445
	.byte	0x8
	.byte	0
	.byte	0x10
	.4byte	0x12a
	.byte	0x9
	.byte	0x4
	.4byte	0x41f
	.byte	0x15
	.byte	0x9
	.byte	0x4
	.4byte	0x42a
	.byte	0xa
	.4byte	0x3b8
	.4byte	0x445
	.byte	0xb
	.4byte	0x3bf
	.byte	0xb
	.4byte	0x3c5
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x431
	.byte	0x3
	.4byte	.LASF64
	.byte	0x7
	.byte	0x28
	.byte	0x3
	.4byte	0x3ee
	.byte	0x6
	.4byte	0x44b
	.byte	0xf
	.4byte	.LASF65
	.byte	0x7
	.byte	0x2a
	.byte	0x1f
	.4byte	0x457
	.byte	0x16
	.4byte	.LASF350
	.byte	0x7
	.byte	0x4
	.4byte	0xa0
	.byte	0x8
	.byte	0x32
	.byte	0xe
	.4byte	0x5bf
	.byte	0x8
	.4byte	.LASF66
	.byte	0x2
	.byte	0x8
	.4byte	.LASF67
	.byte	0x3
	.byte	0x8
	.4byte	.LASF68
	.byte	0x5
	.byte	0x8
	.4byte	.LASF69
	.byte	0x8
	.byte	0x8
	.4byte	.LASF70
	.byte	0x9
	.byte	0x8
	.4byte	.LASF71
	.byte	0xc
	.byte	0x8
	.4byte	.LASF72
	.byte	0xe
	.byte	0x8
	.4byte	.LASF73
	.byte	0x10
	.byte	0x8
	.4byte	.LASF74
	.byte	0x11
	.byte	0x8
	.4byte	.LASF75
	.byte	0x12
	.byte	0x8
	.4byte	.LASF76
	.byte	0x13
	.byte	0x8
	.4byte	.LASF77
	.byte	0x14
	.byte	0x8
	.4byte	.LASF78
	.byte	0x15
	.byte	0x8
	.4byte	.LASF79
	.byte	0x16
	.byte	0x8
	.4byte	.LASF80
	.byte	0x17
	.byte	0x8
	.4byte	.LASF81
	.byte	0x18
	.byte	0x8
	.4byte	.LASF82
	.byte	0x19
	.byte	0x8
	.4byte	.LASF83
	.byte	0x1a
	.byte	0x8
	.4byte	.LASF84
	.byte	0x1b
	.byte	0x8
	.4byte	.LASF85
	.byte	0x1c
	.byte	0x8
	.4byte	.LASF86
	.byte	0x1d
	.byte	0x8
	.4byte	.LASF87
	.byte	0x1e
	.byte	0x8
	.4byte	.LASF88
	.byte	0x1f
	.byte	0x8
	.4byte	.LASF89
	.byte	0x20
	.byte	0x8
	.4byte	.LASF90
	.byte	0x21
	.byte	0x8
	.4byte	.LASF91
	.byte	0x22
	.byte	0x8
	.4byte	.LASF92
	.byte	0x23
	.byte	0x8
	.4byte	.LASF93
	.byte	0x24
	.byte	0x8
	.4byte	.LASF94
	.byte	0x25
	.byte	0x8
	.4byte	.LASF95
	.byte	0x26
	.byte	0x8
	.4byte	.LASF96
	.byte	0x27
	.byte	0x8
	.4byte	.LASF97
	.byte	0x28
	.byte	0x8
	.4byte	.LASF98
	.byte	0x29
	.byte	0x8
	.4byte	.LASF99
	.byte	0x2a
	.byte	0x8
	.4byte	.LASF100
	.byte	0x2b
	.byte	0x8
	.4byte	.LASF101
	.byte	0x2c
	.byte	0x8
	.4byte	.LASF102
	.byte	0x2d
	.byte	0x8
	.4byte	.LASF103
	.byte	0x2e
	.byte	0x8
	.4byte	.LASF104
	.byte	0x2f
	.byte	0x8
	.4byte	.LASF105
	.byte	0x30
	.byte	0x8
	.4byte	.LASF106
	.byte	0x31
	.byte	0x8
	.4byte	.LASF107
	.byte	0x32
	.byte	0x8
	.4byte	.LASF108
	.byte	0x33
	.byte	0x8
	.4byte	.LASF109
	.byte	0x34
	.byte	0x8
	.4byte	.LASF110
	.byte	0x35
	.byte	0x8
	.4byte	.LASF111
	.byte	0x36
	.byte	0x8
	.4byte	.LASF112
	.byte	0x37
	.byte	0x8
	.4byte	.LASF113
	.byte	0x38
	.byte	0x8
	.4byte	.LASF114
	.byte	0x39
	.byte	0x8
	.4byte	.LASF115
	.byte	0x3a
	.byte	0x8
	.4byte	.LASF116
	.byte	0x3b
	.byte	0x8
	.4byte	.LASF117
	.byte	0x3c
	.byte	0x8
	.4byte	.LASF118
	.byte	0x3d
	.byte	0x8
	.4byte	.LASF119
	.byte	0x3e
	.byte	0
	.byte	0x3
	.4byte	.LASF120
	.byte	0x8
	.byte	0x85
	.byte	0x3
	.4byte	0x468
	.byte	0x7
	.byte	0x7
	.byte	0x4
	.4byte	0xa0
	.byte	0x2
	.byte	0x47
	.byte	0xe
	.4byte	0x5e6
	.byte	0x8
	.4byte	.LASF121
	.byte	0
	.byte	0x8
	.4byte	.LASF122
	.byte	0x1
	.byte	0
	.byte	0x12
	.2byte	0xd14
	.byte	0x2
	.byte	0x4e
	.byte	0x9
	.4byte	0x729
	.byte	0xe
	.string	"ISR"
	.byte	0x2
	.byte	0x4f
	.byte	0x11
	.4byte	0x73e
	.byte	0
	.byte	0xe
	.string	"IPR"
	.byte	0x2
	.byte	0x50
	.byte	0x11
	.4byte	0x743
	.byte	0x20
	.byte	0xd
	.4byte	.LASF123
	.byte	0x2
	.byte	0x51
	.byte	0x11
	.4byte	0xed
	.byte	0x40
	.byte	0xd
	.4byte	.LASF124
	.byte	0x2
	.byte	0x52
	.byte	0x11
	.4byte	0xed
	.byte	0x44
	.byte	0xd
	.4byte	.LASF125
	.byte	0x2
	.byte	0x53
	.byte	0x11
	.4byte	0xed
	.byte	0x48
	.byte	0xd
	.4byte	.LASF126
	.byte	0x2
	.byte	0x54
	.byte	0x11
	.4byte	0xf2
	.byte	0x4c
	.byte	0xd
	.4byte	.LASF127
	.byte	0x2
	.byte	0x55
	.byte	0x10
	.4byte	0x758
	.byte	0x50
	.byte	0xd
	.4byte	.LASF128
	.byte	0x2
	.byte	0x56
	.byte	0xb
	.4byte	0x75d
	.byte	0x54
	.byte	0xd
	.4byte	.LASF129
	.byte	0x2
	.byte	0x57
	.byte	0x11
	.4byte	0x77d
	.byte	0x60
	.byte	0xd
	.4byte	.LASF130
	.byte	0x2
	.byte	0x58
	.byte	0xb
	.4byte	0x782
	.byte	0x70
	.byte	0x17
	.4byte	.LASF131
	.byte	0x2
	.byte	0x59
	.byte	0x11
	.4byte	0x7a2
	.2byte	0x100
	.byte	0x17
	.4byte	.LASF132
	.byte	0x2
	.byte	0x5a
	.byte	0xb
	.4byte	0x7a7
	.2byte	0x120
	.byte	0x17
	.4byte	.LASF133
	.byte	0x2
	.byte	0x5b
	.byte	0x11
	.4byte	0x7a2
	.2byte	0x180
	.byte	0x17
	.4byte	.LASF134
	.byte	0x2
	.byte	0x5c
	.byte	0xb
	.4byte	0x7a7
	.2byte	0x1a0
	.byte	0x17
	.4byte	.LASF135
	.byte	0x2
	.byte	0x5d
	.byte	0x11
	.4byte	0x7a2
	.2byte	0x200
	.byte	0x17
	.4byte	.LASF136
	.byte	0x2
	.byte	0x5e
	.byte	0xb
	.4byte	0x7a7
	.2byte	0x220
	.byte	0x17
	.4byte	.LASF137
	.byte	0x2
	.byte	0x5f
	.byte	0x11
	.4byte	0x7a2
	.2byte	0x280
	.byte	0x17
	.4byte	.LASF138
	.byte	0x2
	.byte	0x60
	.byte	0xb
	.4byte	0x7a7
	.2byte	0x2a0
	.byte	0x17
	.4byte	.LASF139
	.byte	0x2
	.byte	0x61
	.byte	0x11
	.4byte	0x7a2
	.2byte	0x300
	.byte	0x17
	.4byte	.LASF140
	.byte	0x2
	.byte	0x62
	.byte	0xb
	.4byte	0x7b7
	.2byte	0x320
	.byte	0x17
	.4byte	.LASF141
	.byte	0x2
	.byte	0x63
	.byte	0x10
	.4byte	0x7d7
	.2byte	0x400
	.byte	0x17
	.4byte	.LASF142
	.byte	0x2
	.byte	0x64
	.byte	0xb
	.4byte	0x7dc
	.2byte	0x500
	.byte	0x17
	.4byte	.LASF143
	.byte	0x2
	.byte	0x65
	.byte	0x11
	.4byte	0xed
	.2byte	0xd10
	.byte	0
	.byte	0x13
	.4byte	0xf2
	.4byte	0x739
	.byte	0x14
	.4byte	0xa0
	.byte	0x7
	.byte	0
	.byte	0x6
	.4byte	0x729
	.byte	0x5
	.4byte	0x739
	.byte	0x5
	.4byte	0x739
	.byte	0x13
	.4byte	0xb3
	.4byte	0x758
	.byte	0x14
	.4byte	0xa0
	.byte	0x3
	.byte	0
	.byte	0x5
	.4byte	0x748
	.byte	0x13
	.4byte	0xa7
	.4byte	0x76d
	.byte	0x14
	.4byte	0xa0
	.byte	0xb
	.byte	0
	.byte	0x13
	.4byte	0xed
	.4byte	0x77d
	.byte	0x14
	.4byte	0xa0
	.byte	0x3
	.byte	0
	.byte	0x5
	.4byte	0x76d
	.byte	0x13
	.4byte	0xa7
	.4byte	0x792
	.byte	0x14
	.4byte	0xa0
	.byte	0x8f
	.byte	0
	.byte	0x13
	.4byte	0xed
	.4byte	0x7a2
	.byte	0x14
	.4byte	0xa0
	.byte	0x7
	.byte	0
	.byte	0x5
	.4byte	0x792
	.byte	0x13
	.4byte	0xa7
	.4byte	0x7b7
	.byte	0x14
	.4byte	0xa0
	.byte	0x5f
	.byte	0
	.byte	0x13
	.4byte	0xa7
	.4byte	0x7c7
	.byte	0x14
	.4byte	0xa0
	.byte	0xdf
	.byte	0
	.byte	0x13
	.4byte	0xb3
	.4byte	0x7d7
	.byte	0x14
	.4byte	0xa0
	.byte	0xff
	.byte	0
	.byte	0x5
	.4byte	0x7c7
	.byte	0x13
	.4byte	0xa7
	.4byte	0x7ed
	.byte	0x18
	.4byte	0xa0
	.2byte	0x80f
	.byte	0
	.byte	0x3
	.4byte	.LASF144
	.byte	0x2
	.byte	0x66
	.byte	0x2
	.4byte	0x5e6
	.byte	0xf
	.4byte	.LASF145
	.byte	0x9
	.byte	0xc
	.byte	0x11
	.4byte	0xe1
	.byte	0xc
	.byte	0x50
	.byte	0x8
	.byte	0x9e
	.byte	0x9
	.4byte	0x913
	.byte	0xd
	.4byte	.LASF146
	.byte	0x8
	.byte	0xa0
	.byte	0x13
	.4byte	0xed
	.byte	0
	.byte	0xd
	.4byte	.LASF147
	.byte	0x8
	.byte	0xa1
	.byte	0x13
	.4byte	0xed
	.byte	0x4
	.byte	0xd
	.4byte	.LASF148
	.byte	0x8
	.byte	0xa2
	.byte	0x13
	.4byte	0xed
	.byte	0x8
	.byte	0xd
	.4byte	.LASF149
	.byte	0x8
	.byte	0xa3
	.byte	0x13
	.4byte	0xed
	.byte	0xc
	.byte	0xd
	.4byte	.LASF150
	.byte	0x8
	.byte	0xa4
	.byte	0x13
	.4byte	0xed
	.byte	0x10
	.byte	0xd
	.4byte	.LASF151
	.byte	0x8
	.byte	0xa5
	.byte	0x13
	.4byte	0xed
	.byte	0x14
	.byte	0xd
	.4byte	.LASF152
	.byte	0x8
	.byte	0xa6
	.byte	0x13
	.4byte	0xed
	.byte	0x18
	.byte	0xd
	.4byte	.LASF153
	.byte	0x8
	.byte	0xa7
	.byte	0x13
	.4byte	0xed
	.byte	0x1c
	.byte	0xd
	.4byte	.LASF154
	.byte	0x8
	.byte	0xa8
	.byte	0x13
	.4byte	0xed
	.byte	0x20
	.byte	0xd
	.4byte	.LASF155
	.byte	0x8
	.byte	0xa9
	.byte	0x13
	.4byte	0xed
	.byte	0x24
	.byte	0xd
	.4byte	.LASF156
	.byte	0x8
	.byte	0xaa
	.byte	0x13
	.4byte	0xed
	.byte	0x28
	.byte	0xd
	.4byte	.LASF157
	.byte	0x8
	.byte	0xab
	.byte	0x13
	.4byte	0xed
	.byte	0x2c
	.byte	0xd
	.4byte	.LASF158
	.byte	0x8
	.byte	0xac
	.byte	0x13
	.4byte	0xed
	.byte	0x30
	.byte	0xd
	.4byte	.LASF159
	.byte	0x8
	.byte	0xad
	.byte	0x13
	.4byte	0xed
	.byte	0x34
	.byte	0xd
	.4byte	.LASF160
	.byte	0x8
	.byte	0xae
	.byte	0x13
	.4byte	0xed
	.byte	0x38
	.byte	0xd
	.4byte	.LASF161
	.byte	0x8
	.byte	0xaf
	.byte	0x13
	.4byte	0xed
	.byte	0x3c
	.byte	0xd
	.4byte	.LASF162
	.byte	0x8
	.byte	0xb0
	.byte	0x13
	.4byte	0xed
	.byte	0x40
	.byte	0xd
	.4byte	.LASF163
	.byte	0x8
	.byte	0xb1
	.byte	0x13
	.4byte	0xed
	.byte	0x44
	.byte	0xd
	.4byte	.LASF164
	.byte	0x8
	.byte	0xb2
	.byte	0x13
	.4byte	0xed
	.byte	0x48
	.byte	0xd
	.4byte	.LASF165
	.byte	0x8
	.byte	0xb3
	.byte	0x13
	.4byte	0xed
	.byte	0x4c
	.byte	0
	.byte	0x3
	.4byte	.LASF166
	.byte	0x8
	.byte	0xb4
	.byte	0x3
	.4byte	0x805
	.byte	0x19
	.byte	0x10
	.byte	0x8
	.2byte	0x155
	.byte	0x9
	.4byte	0x962
	.byte	0x1a
	.4byte	.LASF125
	.byte	0x8
	.2byte	0x157
	.byte	0x13
	.4byte	0xed
	.byte	0
	.byte	0x1a
	.4byte	.LASF167
	.byte	0x8
	.2byte	0x158
	.byte	0x13
	.4byte	0xed
	.byte	0x4
	.byte	0x1a
	.4byte	.LASF168
	.byte	0x8
	.2byte	0x159
	.byte	0x13
	.4byte	0xed
	.byte	0x8
	.byte	0x1a
	.4byte	.LASF169
	.byte	0x8
	.2byte	0x15a
	.byte	0x13
	.4byte	0xed
	.byte	0xc
	.byte	0
	.byte	0x1b
	.4byte	.LASF170
	.byte	0x8
	.2byte	0x15b
	.byte	0x3
	.4byte	0x91f
	.byte	0x19
	.byte	0x8
	.byte	0x8
	.2byte	0x15e
	.byte	0x9
	.4byte	0x996
	.byte	0x1a
	.4byte	.LASF171
	.byte	0x8
	.2byte	0x160
	.byte	0x13
	.4byte	0xed
	.byte	0
	.byte	0x1a
	.4byte	.LASF172
	.byte	0x8
	.2byte	0x161
	.byte	0x13
	.4byte	0xed
	.byte	0x4
	.byte	0
	.byte	0x1b
	.4byte	.LASF173
	.byte	0x8
	.2byte	0x162
	.byte	0x3
	.4byte	0x96f
	.byte	0x19
	.byte	0x1c
	.byte	0x8
	.2byte	0x18c
	.byte	0x9
	.4byte	0xa10
	.byte	0x1a
	.4byte	.LASF174
	.byte	0x8
	.2byte	0x18e
	.byte	0x13
	.4byte	0xed
	.byte	0
	.byte	0x1a
	.4byte	.LASF175
	.byte	0x8
	.2byte	0x18f
	.byte	0x13
	.4byte	0xed
	.byte	0x4
	.byte	0x1a
	.4byte	.LASF176
	.byte	0x8
	.2byte	0x190
	.byte	0x13
	.4byte	0xed
	.byte	0x8
	.byte	0x1a
	.4byte	.LASF177
	.byte	0x8
	.2byte	0x191
	.byte	0x13
	.4byte	0xed
	.byte	0xc
	.byte	0x1a
	.4byte	.LASF178
	.byte	0x8
	.2byte	0x192
	.byte	0x13
	.4byte	0xed
	.byte	0x10
	.byte	0x1c
	.string	"BCR"
	.byte	0x8
	.2byte	0x193
	.byte	0x13
	.4byte	0xed
	.byte	0x14
	.byte	0x1a
	.4byte	.LASF179
	.byte	0x8
	.2byte	0x194
	.byte	0x13
	.4byte	0xed
	.byte	0x18
	.byte	0
	.byte	0x1b
	.4byte	.LASF180
	.byte	0x8
	.2byte	0x195
	.byte	0x3
	.4byte	0x9a3
	.byte	0x19
	.byte	0x30
	.byte	0x8
	.2byte	0x1c9
	.byte	0x9
	.4byte	0xad0
	.byte	0x1a
	.4byte	.LASF181
	.byte	0x8
	.2byte	0x1cb
	.byte	0x13
	.4byte	0xed
	.byte	0
	.byte	0x1a
	.4byte	.LASF182
	.byte	0x8
	.2byte	0x1cc
	.byte	0x13
	.4byte	0xed
	.byte	0x4
	.byte	0x1a
	.4byte	.LASF183
	.byte	0x8
	.2byte	0x1cd
	.byte	0x13
	.4byte	0xed
	.byte	0x8
	.byte	0x1a
	.4byte	.LASF184
	.byte	0x8
	.2byte	0x1ce
	.byte	0x13
	.4byte	0xed
	.byte	0xc
	.byte	0x1a
	.4byte	.LASF185
	.byte	0x8
	.2byte	0x1cf
	.byte	0x13
	.4byte	0xed
	.byte	0x10
	.byte	0x1a
	.4byte	.LASF186
	.byte	0x8
	.2byte	0x1d0
	.byte	0x13
	.4byte	0xed
	.byte	0x14
	.byte	0x1a
	.4byte	.LASF187
	.byte	0x8
	.2byte	0x1d1
	.byte	0x13
	.4byte	0xed
	.byte	0x18
	.byte	0x1a
	.4byte	.LASF188
	.byte	0x8
	.2byte	0x1d2
	.byte	0x13
	.4byte	0xed
	.byte	0x1c
	.byte	0x1a
	.4byte	.LASF189
	.byte	0x8
	.2byte	0x1d3
	.byte	0x13
	.4byte	0xed
	.byte	0x20
	.byte	0x1a
	.4byte	.LASF190
	.byte	0x8
	.2byte	0x1d4
	.byte	0x13
	.4byte	0xed
	.byte	0x24
	.byte	0x1a
	.4byte	.LASF191
	.byte	0x8
	.2byte	0x1d6
	.byte	0x13
	.4byte	0xed
	.byte	0x28
	.byte	0x1a
	.4byte	.LASF192
	.byte	0x8
	.2byte	0x1d7
	.byte	0x13
	.4byte	0xed
	.byte	0x2c
	.byte	0
	.byte	0x1b
	.4byte	.LASF193
	.byte	0x8
	.2byte	0x1d8
	.byte	0x3
	.4byte	0xa1d
	.byte	0x3
	.4byte	.LASF194
	.byte	0xa
	.byte	0x22
	.byte	0x19
	.4byte	0xae9
	.byte	0x9
	.byte	0x4
	.4byte	0xaef
	.byte	0x1d
	.4byte	.LASF301
	.byte	0x3
	.4byte	.LASF195
	.byte	0xb
	.byte	0x2c
	.byte	0xe
	.4byte	0x71
	.byte	0x3
	.4byte	.LASF196
	.byte	0xb
	.byte	0x72
	.byte	0xe
	.4byte	0x71
	.byte	0x3
	.4byte	.LASF197
	.byte	0xb
	.byte	0x91
	.byte	0x17
	.4byte	0x99
	.byte	0x1b
	.4byte	.LASF198
	.byte	0xc
	.2byte	0x165
	.byte	0x17
	.4byte	0xa0
	.byte	0x1e
	.byte	0x4
	.byte	0xb
	.byte	0xa6
	.byte	0x3
	.4byte	0xb47
	.byte	0x1f
	.4byte	.LASF199
	.byte	0xb
	.byte	0xa8
	.byte	0xc
	.4byte	0xb18
	.byte	0x1f
	.4byte	.LASF200
	.byte	0xb
	.byte	0xa9
	.byte	0x13
	.4byte	0xb47
	.byte	0
	.byte	0x13
	.4byte	0x38
	.4byte	0xb57
	.byte	0x14
	.4byte	0xa0
	.byte	0x3
	.byte	0
	.byte	0xc
	.byte	0x8
	.byte	0xb
	.byte	0xa3
	.byte	0x9
	.4byte	0xb7b
	.byte	0xd
	.4byte	.LASF201
	.byte	0xb
	.byte	0xa5
	.byte	0x7
	.4byte	0x99
	.byte	0
	.byte	0xd
	.4byte	.LASF202
	.byte	0xb
	.byte	0xaa
	.byte	0x5
	.4byte	0xb25
	.byte	0x4
	.byte	0
	.byte	0x3
	.4byte	.LASF203
	.byte	0xb
	.byte	0xab
	.byte	0x3
	.4byte	0xb57
	.byte	0x3
	.4byte	.LASF204
	.byte	0xb
	.byte	0xaf
	.byte	0x1b
	.4byte	0xadd
	.byte	0x20
	.byte	0x4
	.byte	0x3
	.4byte	.LASF205
	.byte	0xd
	.byte	0x16
	.byte	0x19
	.4byte	0x84
	.byte	0x21
	.4byte	.LASF210
	.byte	0x18
	.byte	0xd
	.byte	0x2f
	.byte	0x8
	.4byte	0xbfb
	.byte	0xd
	.4byte	.LASF206
	.byte	0xd
	.byte	0x31
	.byte	0x13
	.4byte	0xbfb
	.byte	0
	.byte	0xe
	.string	"_k"
	.byte	0xd
	.byte	0x32
	.byte	0x7
	.4byte	0x99
	.byte	0x4
	.byte	0xd
	.4byte	.LASF207
	.byte	0xd
	.byte	0x32
	.byte	0xb
	.4byte	0x99
	.byte	0x8
	.byte	0xd
	.4byte	.LASF208
	.byte	0xd
	.byte	0x32
	.byte	0x14
	.4byte	0x99
	.byte	0xc
	.byte	0xd
	.4byte	.LASF209
	.byte	0xd
	.byte	0x32
	.byte	0x1b
	.4byte	0x99
	.byte	0x10
	.byte	0xe
	.string	"_x"
	.byte	0xd
	.byte	0x33
	.byte	0xb
	.4byte	0xc01
	.byte	0x14
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0xba1
	.byte	0x13
	.4byte	0xb95
	.4byte	0xc11
	.byte	0x14
	.4byte	0xa0
	.byte	0
	.byte	0
	.byte	0x21
	.4byte	.LASF211
	.byte	0x24
	.byte	0xd
	.byte	0x37
	.byte	0x8
	.4byte	0xc94
	.byte	0xd
	.4byte	.LASF212
	.byte	0xd
	.byte	0x39
	.byte	0x9
	.4byte	0x99
	.byte	0
	.byte	0xd
	.4byte	.LASF213
	.byte	0xd
	.byte	0x3a
	.byte	0x9
	.4byte	0x99
	.byte	0x4
	.byte	0xd
	.4byte	.LASF214
	.byte	0xd
	.byte	0x3b
	.byte	0x9
	.4byte	0x99
	.byte	0x8
	.byte	0xd
	.4byte	.LASF215
	.byte	0xd
	.byte	0x3c
	.byte	0x9
	.4byte	0x99
	.byte	0xc
	.byte	0xd
	.4byte	.LASF216
	.byte	0xd
	.byte	0x3d
	.byte	0x9
	.4byte	0x99
	.byte	0x10
	.byte	0xd
	.4byte	.LASF217
	.byte	0xd
	.byte	0x3e
	.byte	0x9
	.4byte	0x99
	.byte	0x14
	.byte	0xd
	.4byte	.LASF218
	.byte	0xd
	.byte	0x3f
	.byte	0x9
	.4byte	0x99
	.byte	0x18
	.byte	0xd
	.4byte	.LASF219
	.byte	0xd
	.byte	0x40
	.byte	0x9
	.4byte	0x99
	.byte	0x1c
	.byte	0xd
	.4byte	.LASF220
	.byte	0xd
	.byte	0x41
	.byte	0x9
	.4byte	0x99
	.byte	0x20
	.byte	0
	.byte	0x22
	.4byte	.LASF221
	.2byte	0x108
	.byte	0xd
	.byte	0x4a
	.byte	0x8
	.4byte	0xcd9
	.byte	0xd
	.4byte	.LASF222
	.byte	0xd
	.byte	0x4b
	.byte	0xa
	.4byte	0xcd9
	.byte	0
	.byte	0xd
	.4byte	.LASF223
	.byte	0xd
	.byte	0x4c
	.byte	0x9
	.4byte	0xcd9
	.byte	0x80
	.byte	0x17
	.4byte	.LASF224
	.byte	0xd
	.byte	0x4e
	.byte	0xa
	.4byte	0xb95
	.2byte	0x100
	.byte	0x17
	.4byte	.LASF225
	.byte	0xd
	.byte	0x51
	.byte	0xa
	.4byte	0xb95
	.2byte	0x104
	.byte	0
	.byte	0x13
	.4byte	0xb93
	.4byte	0xce9
	.byte	0x14
	.4byte	0xa0
	.byte	0x1f
	.byte	0
	.byte	0x22
	.4byte	.LASF226
	.2byte	0x190
	.byte	0xd
	.byte	0x5d
	.byte	0x8
	.4byte	0xd2c
	.byte	0xd
	.4byte	.LASF206
	.byte	0xd
	.byte	0x5e
	.byte	0x12
	.4byte	0xd2c
	.byte	0
	.byte	0xd
	.4byte	.LASF227
	.byte	0xd
	.byte	0x5f
	.byte	0x6
	.4byte	0x99
	.byte	0x4
	.byte	0xd
	.4byte	.LASF228
	.byte	0xd
	.byte	0x61
	.byte	0x9
	.4byte	0xd32
	.byte	0x8
	.byte	0xd
	.4byte	.LASF221
	.byte	0xd
	.byte	0x62
	.byte	0x1e
	.4byte	0xc94
	.byte	0x88
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0xce9
	.byte	0x13
	.4byte	0x42b
	.4byte	0xd42
	.byte	0x14
	.4byte	0xa0
	.byte	0x1f
	.byte	0
	.byte	0x21
	.4byte	.LASF229
	.byte	0x8
	.byte	0xd
	.byte	0x75
	.byte	0x8
	.4byte	0xd6a
	.byte	0xd
	.4byte	.LASF230
	.byte	0xd
	.byte	0x76
	.byte	0x11
	.4byte	0xd6a
	.byte	0
	.byte	0xd
	.4byte	.LASF231
	.byte	0xd
	.byte	0x77
	.byte	0x6
	.4byte	0x99
	.byte	0x4
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x38
	.byte	0x21
	.4byte	.LASF232
	.byte	0x68
	.byte	0xd
	.byte	0xb5
	.byte	0x8
	.4byte	0xeb3
	.byte	0xe
	.string	"_p"
	.byte	0xd
	.byte	0xb6
	.byte	0x12
	.4byte	0xd6a
	.byte	0
	.byte	0xe
	.string	"_r"
	.byte	0xd
	.byte	0xb7
	.byte	0x7
	.4byte	0x99
	.byte	0x4
	.byte	0xe
	.string	"_w"
	.byte	0xd
	.byte	0xb8
	.byte	0x7
	.4byte	0x99
	.byte	0x8
	.byte	0xd
	.4byte	.LASF233
	.byte	0xd
	.byte	0xb9
	.byte	0x9
	.4byte	0x4b
	.byte	0xc
	.byte	0xd
	.4byte	.LASF234
	.byte	0xd
	.byte	0xba
	.byte	0x9
	.4byte	0x4b
	.byte	0xe
	.byte	0xe
	.string	"_bf"
	.byte	0xd
	.byte	0xbb
	.byte	0x11
	.4byte	0xd42
	.byte	0x10
	.byte	0xd
	.4byte	.LASF235
	.byte	0xd
	.byte	0xbc
	.byte	0x7
	.4byte	0x99
	.byte	0x18
	.byte	0xd
	.4byte	.LASF236
	.byte	0xd
	.byte	0xc3
	.byte	0xa
	.4byte	0xb93
	.byte	0x1c
	.byte	0xd
	.4byte	.LASF237
	.byte	0xd
	.byte	0xc5
	.byte	0x1d
	.4byte	0x102b
	.byte	0x20
	.byte	0xd
	.4byte	.LASF238
	.byte	0xd
	.byte	0xc7
	.byte	0x1d
	.4byte	0x104f
	.byte	0x24
	.byte	0xd
	.4byte	.LASF239
	.byte	0xd
	.byte	0xca
	.byte	0xd
	.4byte	0x1073
	.byte	0x28
	.byte	0xd
	.4byte	.LASF240
	.byte	0xd
	.byte	0xcb
	.byte	0x9
	.4byte	0x108d
	.byte	0x2c
	.byte	0xe
	.string	"_ub"
	.byte	0xd
	.byte	0xce
	.byte	0x11
	.4byte	0xd42
	.byte	0x30
	.byte	0xe
	.string	"_up"
	.byte	0xd
	.byte	0xcf
	.byte	0x12
	.4byte	0xd6a
	.byte	0x38
	.byte	0xe
	.string	"_ur"
	.byte	0xd
	.byte	0xd0
	.byte	0x7
	.4byte	0x99
	.byte	0x3c
	.byte	0xd
	.4byte	.LASF241
	.byte	0xd
	.byte	0xd3
	.byte	0x11
	.4byte	0x1093
	.byte	0x40
	.byte	0xd
	.4byte	.LASF242
	.byte	0xd
	.byte	0xd4
	.byte	0x11
	.4byte	0x10a3
	.byte	0x43
	.byte	0xe
	.string	"_lb"
	.byte	0xd
	.byte	0xd7
	.byte	0x11
	.4byte	0xd42
	.byte	0x44
	.byte	0xd
	.4byte	.LASF243
	.byte	0xd
	.byte	0xda
	.byte	0x7
	.4byte	0x99
	.byte	0x4c
	.byte	0xd
	.4byte	.LASF244
	.byte	0xd
	.byte	0xdb
	.byte	0xa
	.4byte	0xaf4
	.byte	0x50
	.byte	0xd
	.4byte	.LASF245
	.byte	0xd
	.byte	0xde
	.byte	0x12
	.4byte	0xed1
	.byte	0x54
	.byte	0xd
	.4byte	.LASF246
	.byte	0xd
	.byte	0xe2
	.byte	0xc
	.4byte	0xb87
	.byte	0x58
	.byte	0xd
	.4byte	.LASF247
	.byte	0xd
	.byte	0xe4
	.byte	0xe
	.4byte	0xb7b
	.byte	0x5c
	.byte	0xd
	.4byte	.LASF248
	.byte	0xd
	.byte	0xe5
	.byte	0x9
	.4byte	0x99
	.byte	0x64
	.byte	0
	.byte	0xa
	.4byte	0xb0c
	.4byte	0xed1
	.byte	0xb
	.4byte	0xed1
	.byte	0xb
	.4byte	0xb93
	.byte	0xb
	.4byte	0x1025
	.byte	0xb
	.4byte	0x99
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0xedc
	.byte	0x6
	.4byte	0xed1
	.byte	0x23
	.4byte	.LASF249
	.2byte	0x428
	.byte	0xd
	.2byte	0x239
	.byte	0x8
	.4byte	0x1025
	.byte	0x1a
	.4byte	.LASF250
	.byte	0xd
	.2byte	0x23b
	.byte	0x7
	.4byte	0x99
	.byte	0
	.byte	0x1a
	.4byte	.LASF251
	.byte	0xd
	.2byte	0x240
	.byte	0xb
	.4byte	0x10ff
	.byte	0x4
	.byte	0x1a
	.4byte	.LASF252
	.byte	0xd
	.2byte	0x240
	.byte	0x14
	.4byte	0x10ff
	.byte	0x8
	.byte	0x1a
	.4byte	.LASF253
	.byte	0xd
	.2byte	0x240
	.byte	0x1e
	.4byte	0x10ff
	.byte	0xc
	.byte	0x1a
	.4byte	.LASF254
	.byte	0xd
	.2byte	0x242
	.byte	0x8
	.4byte	0x99
	.byte	0x10
	.byte	0x1a
	.4byte	.LASF255
	.byte	0xd
	.2byte	0x243
	.byte	0x8
	.4byte	0x12ff
	.byte	0x14
	.byte	0x1a
	.4byte	.LASF256
	.byte	0xd
	.2byte	0x246
	.byte	0x7
	.4byte	0x99
	.byte	0x30
	.byte	0x1a
	.4byte	.LASF257
	.byte	0xd
	.2byte	0x247
	.byte	0x16
	.4byte	0x1314
	.byte	0x34
	.byte	0x1a
	.4byte	.LASF258
	.byte	0xd
	.2byte	0x249
	.byte	0x7
	.4byte	0x99
	.byte	0x38
	.byte	0x1a
	.4byte	.LASF259
	.byte	0xd
	.2byte	0x24b
	.byte	0xa
	.4byte	0x1325
	.byte	0x3c
	.byte	0x1a
	.4byte	.LASF260
	.byte	0xd
	.2byte	0x24e
	.byte	0x13
	.4byte	0xbfb
	.byte	0x40
	.byte	0x1a
	.4byte	.LASF261
	.byte	0xd
	.2byte	0x24f
	.byte	0x7
	.4byte	0x99
	.byte	0x44
	.byte	0x1a
	.4byte	.LASF262
	.byte	0xd
	.2byte	0x250
	.byte	0x13
	.4byte	0xbfb
	.byte	0x48
	.byte	0x1a
	.4byte	.LASF263
	.byte	0xd
	.2byte	0x251
	.byte	0x14
	.4byte	0x132b
	.byte	0x4c
	.byte	0x1a
	.4byte	.LASF264
	.byte	0xd
	.2byte	0x254
	.byte	0x7
	.4byte	0x99
	.byte	0x50
	.byte	0x1a
	.4byte	.LASF265
	.byte	0xd
	.2byte	0x255
	.byte	0x9
	.4byte	0x1025
	.byte	0x54
	.byte	0x1a
	.4byte	.LASF266
	.byte	0xd
	.2byte	0x278
	.byte	0x7
	.4byte	0x12da
	.byte	0x58
	.byte	0x24
	.4byte	.LASF226
	.byte	0xd
	.2byte	0x27c
	.byte	0x13
	.4byte	0xd2c
	.2byte	0x148
	.byte	0x24
	.4byte	.LASF267
	.byte	0xd
	.2byte	0x27d
	.byte	0x12
	.4byte	0xce9
	.2byte	0x14c
	.byte	0x24
	.4byte	.LASF268
	.byte	0xd
	.2byte	0x281
	.byte	0xc
	.4byte	0x133c
	.2byte	0x2dc
	.byte	0x24
	.4byte	.LASF269
	.byte	0xd
	.2byte	0x286
	.byte	0x10
	.4byte	0x10c0
	.2byte	0x2e0
	.byte	0x24
	.4byte	.LASF270
	.byte	0xd
	.2byte	0x288
	.byte	0xa
	.4byte	0x1348
	.2byte	0x2ec
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x192
	.byte	0x9
	.byte	0x4
	.4byte	0xeb3
	.byte	0xa
	.4byte	0xb0c
	.4byte	0x104f
	.byte	0xb
	.4byte	0xed1
	.byte	0xb
	.4byte	0xb93
	.byte	0xb
	.4byte	0x18c
	.byte	0xb
	.4byte	0x99
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x1031
	.byte	0xa
	.4byte	0xb00
	.4byte	0x1073
	.byte	0xb
	.4byte	0xed1
	.byte	0xb
	.4byte	0xb93
	.byte	0xb
	.4byte	0xb00
	.byte	0xb
	.4byte	0x99
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x1055
	.byte	0xa
	.4byte	0x99
	.4byte	0x108d
	.byte	0xb
	.4byte	0xed1
	.byte	0xb
	.4byte	0xb93
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x1079
	.byte	0x13
	.4byte	0x38
	.4byte	0x10a3
	.byte	0x14
	.4byte	0xa0
	.byte	0x2
	.byte	0
	.byte	0x13
	.4byte	0x38
	.4byte	0x10b3
	.byte	0x14
	.4byte	0xa0
	.byte	0
	.byte	0
	.byte	0x1b
	.4byte	.LASF271
	.byte	0xd
	.2byte	0x11f
	.byte	0x1a
	.4byte	0xd70
	.byte	0x25
	.4byte	.LASF272
	.byte	0xc
	.byte	0xd
	.2byte	0x123
	.byte	0x8
	.4byte	0x10f9
	.byte	0x1a
	.4byte	.LASF206
	.byte	0xd
	.2byte	0x125
	.byte	0x11
	.4byte	0x10f9
	.byte	0
	.byte	0x1a
	.4byte	.LASF273
	.byte	0xd
	.2byte	0x126
	.byte	0x7
	.4byte	0x99
	.byte	0x4
	.byte	0x1a
	.4byte	.LASF274
	.byte	0xd
	.2byte	0x127
	.byte	0xb
	.4byte	0x10ff
	.byte	0x8
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x10c0
	.byte	0x9
	.byte	0x4
	.4byte	0x10b3
	.byte	0x25
	.4byte	.LASF275
	.byte	0xe
	.byte	0xd
	.2byte	0x13f
	.byte	0x8
	.4byte	0x113e
	.byte	0x1a
	.4byte	.LASF276
	.byte	0xd
	.2byte	0x140
	.byte	0x12
	.4byte	0x113e
	.byte	0
	.byte	0x1a
	.4byte	.LASF277
	.byte	0xd
	.2byte	0x141
	.byte	0x12
	.4byte	0x113e
	.byte	0x6
	.byte	0x1a
	.4byte	.LASF278
	.byte	0xd
	.2byte	0x142
	.byte	0x12
	.4byte	0x5e
	.byte	0xc
	.byte	0
	.byte	0x13
	.4byte	0x5e
	.4byte	0x114e
	.byte	0x14
	.4byte	0xa0
	.byte	0x2
	.byte	0
	.byte	0x19
	.byte	0xd0
	.byte	0xd
	.2byte	0x259
	.byte	0x7
	.4byte	0x1263
	.byte	0x1a
	.4byte	.LASF279
	.byte	0xd
	.2byte	0x25b
	.byte	0x18
	.4byte	0xa0
	.byte	0
	.byte	0x1a
	.4byte	.LASF280
	.byte	0xd
	.2byte	0x25c
	.byte	0x12
	.4byte	0x1025
	.byte	0x4
	.byte	0x1a
	.4byte	.LASF281
	.byte	0xd
	.2byte	0x25d
	.byte	0x10
	.4byte	0x1263
	.byte	0x8
	.byte	0x1a
	.4byte	.LASF282
	.byte	0xd
	.2byte	0x25e
	.byte	0x17
	.4byte	0xc11
	.byte	0x24
	.byte	0x1a
	.4byte	.LASF283
	.byte	0xd
	.2byte	0x25f
	.byte	0xf
	.4byte	0x99
	.byte	0x48
	.byte	0x1a
	.4byte	.LASF284
	.byte	0xd
	.2byte	0x260
	.byte	0x2c
	.4byte	0x92
	.byte	0x50
	.byte	0x1a
	.4byte	.LASF285
	.byte	0xd
	.2byte	0x261
	.byte	0x1a
	.4byte	0x1105
	.byte	0x58
	.byte	0x1a
	.4byte	.LASF286
	.byte	0xd
	.2byte	0x262
	.byte	0x16
	.4byte	0xb7b
	.byte	0x68
	.byte	0x1a
	.4byte	.LASF287
	.byte	0xd
	.2byte	0x263
	.byte	0x16
	.4byte	0xb7b
	.byte	0x70
	.byte	0x1a
	.4byte	.LASF288
	.byte	0xd
	.2byte	0x264
	.byte	0x16
	.4byte	0xb7b
	.byte	0x78
	.byte	0x1a
	.4byte	.LASF289
	.byte	0xd
	.2byte	0x265
	.byte	0x10
	.4byte	0x1273
	.byte	0x80
	.byte	0x1a
	.4byte	.LASF290
	.byte	0xd
	.2byte	0x266
	.byte	0x10
	.4byte	0x1283
	.byte	0x88
	.byte	0x1a
	.4byte	.LASF291
	.byte	0xd
	.2byte	0x267
	.byte	0xf
	.4byte	0x99
	.byte	0xa0
	.byte	0x1a
	.4byte	.LASF292
	.byte	0xd
	.2byte	0x268
	.byte	0x16
	.4byte	0xb7b
	.byte	0xa4
	.byte	0x1a
	.4byte	.LASF293
	.byte	0xd
	.2byte	0x269
	.byte	0x16
	.4byte	0xb7b
	.byte	0xac
	.byte	0x1a
	.4byte	.LASF294
	.byte	0xd
	.2byte	0x26a
	.byte	0x16
	.4byte	0xb7b
	.byte	0xb4
	.byte	0x1a
	.4byte	.LASF295
	.byte	0xd
	.2byte	0x26b
	.byte	0x16
	.4byte	0xb7b
	.byte	0xbc
	.byte	0x1a
	.4byte	.LASF296
	.byte	0xd
	.2byte	0x26c
	.byte	0x16
	.4byte	0xb7b
	.byte	0xc4
	.byte	0x1a
	.4byte	.LASF297
	.byte	0xd
	.2byte	0x26d
	.byte	0x8
	.4byte	0x99
	.byte	0xcc
	.byte	0
	.byte	0x13
	.4byte	0x192
	.4byte	0x1273
	.byte	0x14
	.4byte	0xa0
	.byte	0x19
	.byte	0
	.byte	0x13
	.4byte	0x192
	.4byte	0x1283
	.byte	0x14
	.4byte	0xa0
	.byte	0x7
	.byte	0
	.byte	0x13
	.4byte	0x192
	.4byte	0x1293
	.byte	0x14
	.4byte	0xa0
	.byte	0x17
	.byte	0
	.byte	0x19
	.byte	0xf0
	.byte	0xd
	.2byte	0x272
	.byte	0x7
	.4byte	0x12ba
	.byte	0x1a
	.4byte	.LASF298
	.byte	0xd
	.2byte	0x275
	.byte	0x1b
	.4byte	0x12ba
	.byte	0
	.byte	0x1a
	.4byte	.LASF299
	.byte	0xd
	.2byte	0x276
	.byte	0x18
	.4byte	0x12ca
	.byte	0x78
	.byte	0
	.byte	0x13
	.4byte	0xd6a
	.4byte	0x12ca
	.byte	0x14
	.4byte	0xa0
	.byte	0x1d
	.byte	0
	.byte	0x13
	.4byte	0xa0
	.4byte	0x12da
	.byte	0x14
	.4byte	0xa0
	.byte	0x1d
	.byte	0
	.byte	0x26
	.byte	0xf0
	.byte	0xd
	.2byte	0x257
	.byte	0x3
	.4byte	0x12ff
	.byte	0x27
	.4byte	.LASF249
	.byte	0xd
	.2byte	0x26e
	.byte	0xb
	.4byte	0x114e
	.byte	0x27
	.4byte	.LASF300
	.byte	0xd
	.2byte	0x277
	.byte	0xb
	.4byte	0x1293
	.byte	0
	.byte	0x13
	.4byte	0x192
	.4byte	0x130f
	.byte	0x14
	.4byte	0xa0
	.byte	0x18
	.byte	0
	.byte	0x1d
	.4byte	.LASF302
	.byte	0x9
	.byte	0x4
	.4byte	0x130f
	.byte	0x11
	.4byte	0x1325
	.byte	0xb
	.4byte	0xed1
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x131a
	.byte	0x9
	.byte	0x4
	.4byte	0xbfb
	.byte	0x11
	.4byte	0x133c
	.byte	0xb
	.4byte	0x99
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0x1342
	.byte	0x9
	.byte	0x4
	.4byte	0x1331
	.byte	0x13
	.4byte	0x10b3
	.4byte	0x1358
	.byte	0x14
	.4byte	0xa0
	.byte	0x2
	.byte	0
	.byte	0x28
	.4byte	.LASF303
	.byte	0xd
	.2byte	0x307
	.byte	0x17
	.4byte	0xed1
	.byte	0x28
	.4byte	.LASF304
	.byte	0xd
	.2byte	0x308
	.byte	0x1d
	.4byte	0xed7
	.byte	0x7
	.byte	0x7
	.byte	0x4
	.4byte	0xa0
	.byte	0xe
	.byte	0x18
	.byte	0x1
	.4byte	0x1393
	.byte	0x8
	.4byte	.LASF305
	.byte	0x1
	.byte	0x8
	.4byte	.LASF306
	.byte	0x2
	.byte	0x8
	.4byte	.LASF307
	.byte	0x3
	.byte	0
	.byte	0x3
	.4byte	.LASF308
	.byte	0xe
	.byte	0x1c
	.byte	0x3
	.4byte	0x1372
	.byte	0x7
	.byte	0x7
	.byte	0x4
	.4byte	0xa0
	.byte	0xe
	.byte	0x20
	.byte	0x1
	.4byte	0x13de
	.byte	0x8
	.4byte	.LASF309
	.byte	0
	.byte	0x8
	.4byte	.LASF310
	.byte	0x4
	.byte	0x8
	.4byte	.LASF311
	.byte	0x28
	.byte	0x8
	.4byte	.LASF312
	.byte	0x48
	.byte	0x8
	.4byte	.LASF313
	.byte	0x14
	.byte	0x8
	.4byte	.LASF314
	.byte	0x10
	.byte	0x8
	.4byte	.LASF315
	.byte	0x1c
	.byte	0x8
	.4byte	.LASF316
	.byte	0x18
	.byte	0
	.byte	0x3
	.4byte	.LASF317
	.byte	0xe
	.byte	0x29
	.byte	0x3
	.4byte	0x139f
	.byte	0xc
	.byte	0xc
	.byte	0xe
	.byte	0x2c
	.byte	0x9
	.4byte	0x141b
	.byte	0xd
	.4byte	.LASF318
	.byte	0xe
	.byte	0x2e
	.byte	0xe
	.4byte	0xc4
	.byte	0
	.byte	0xd
	.4byte	.LASF319
	.byte	0xe
	.byte	0x31
	.byte	0x17
	.4byte	0x1393
	.byte	0x4
	.byte	0xd
	.4byte	.LASF320
	.byte	0xe
	.byte	0x34
	.byte	0x16
	.4byte	0x13de
	.byte	0x8
	.byte	0
	.byte	0x3
	.4byte	.LASF321
	.byte	0xe
	.byte	0x36
	.byte	0x3
	.4byte	0x13ea
	.byte	0x29
	.4byte	.LASF322
	.byte	0x1
	.byte	0x1e
	.byte	0x19
	.4byte	0xb3
	.byte	0x5
	.byte	0x3
	.4byte	s_on
	.byte	0x29
	.4byte	.LASF323
	.byte	0x1
	.byte	0x1f
	.byte	0x10
	.4byte	0xb8
	.byte	0x5
	.byte	0x3
	.4byte	s_cal
	.byte	0x29
	.4byte	.LASF324
	.byte	0x1
	.byte	0x20
	.byte	0xd
	.4byte	0x31e
	.byte	0x5
	.byte	0x3
	.4byte	s_pp
	.byte	0x13
	.4byte	0xc4
	.4byte	0x1473
	.byte	0x14
	.4byte	0xa0
	.byte	0x9
	.byte	0x14
	.4byte	0xa0
	.byte	0x3b
	.byte	0
	.byte	0x2a
	.string	"s_q"
	.byte	0x1
	.byte	0x21
	.byte	0x11
	.4byte	0x145d
	.byte	0x5
	.byte	0x3
	.4byte	s_q
	.byte	0x29
	.4byte	.LASF325
	.byte	0x1
	.byte	0x22
	.byte	0x19
	.4byte	0xb3
	.byte	0x5
	.byte	0x3
	.4byte	s_q_w
	.byte	0x29
	.4byte	.LASF326
	.byte	0x1
	.byte	0x23
	.byte	0x19
	.4byte	0xb3
	.byte	0x5
	.byte	0x3
	.4byte	s_q_r
	.byte	0x29
	.4byte	.LASF327
	.byte	0x1
	.byte	0x24
	.byte	0x19
	.4byte	0xb3
	.byte	0x5
	.byte	0x3
	.4byte	s_q_n
	.byte	0x29
	.4byte	.LASF328
	.byte	0x1
	.byte	0x25
	.byte	0x1a
	.4byte	0xed
	.byte	0x5
	.byte	0x3
	.4byte	s_q_drop
	.byte	0x2b
	.4byte	0x45c
	.byte	0x1
	.2byte	0x185
	.byte	0x18
	.byte	0x5
	.byte	0x3
	.4byte	drv_adc
	.byte	0x2c
	.4byte	.LASF359
	.byte	0x1
	.2byte	0x174
	.byte	0x6
	.4byte	.LFB44
	.4byte	.LFE44-.LFB44
	.byte	0x1
	.byte	0x9c
	.4byte	0x158d
	.byte	0x2d
	.4byte	.LASF329
	.byte	0x1
	.2byte	0x176
	.byte	0xe
	.4byte	0xe1
	.4byte	.LLST35
	.byte	0x2e
	.4byte	0x1620
	.4byte	.LBB114
	.4byte	.LBE114-.LBB114
	.byte	0x1
	.2byte	0x17c
	.byte	0x9
	.4byte	0x154a
	.byte	0x2f
	.4byte	0x162e
	.4byte	.LLST36
	.byte	0x30
	.4byte	.LBB115
	.4byte	.LBE115-.LBB115
	.byte	0x31
	.4byte	0x163b
	.byte	0x31
	.4byte	0x1648
	.byte	0x32
	.4byte	.LVL93
	.4byte	0x19cc
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x5
	.byte	0x3
	.4byte	.LANCHOR7
	.byte	0
	.byte	0
	.byte	0
	.byte	0x34
	.4byte	0x1620
	.4byte	.LBB116
	.4byte	.LBE116-.LBB116
	.byte	0x1
	.2byte	0x181
	.byte	0x9
	.byte	0x2f
	.4byte	0x162e
	.4byte	.LLST37
	.byte	0x30
	.4byte	.LBB117
	.4byte	.LBE117-.LBB117
	.byte	0x31
	.4byte	0x163b
	.byte	0x31
	.4byte	0x1648
	.byte	0x32
	.4byte	.LVL96
	.4byte	0x19cc
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x5
	.byte	0x3
	.4byte	.LANCHOR7+120
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0x35
	.4byte	.LASF336
	.byte	0x1
	.2byte	0x142
	.byte	0xd
	.4byte	0x3b8
	.byte	0x1
	.4byte	0x1610
	.byte	0x36
	.string	"out"
	.byte	0x1
	.2byte	0x142
	.byte	0x23
	.4byte	0x3bf
	.byte	0x37
	.4byte	.LASF330
	.byte	0x1
	.2byte	0x142
	.byte	0x32
	.4byte	0x3c5
	.byte	0x38
	.4byte	.LASF331
	.byte	0x1
	.2byte	0x144
	.byte	0xe
	.4byte	0xe1
	.byte	0x39
	.string	"raw"
	.byte	0x1
	.2byte	0x145
	.byte	0xe
	.4byte	0x1610
	.byte	0x38
	.4byte	.LASF332
	.byte	0x1
	.2byte	0x146
	.byte	0xe
	.4byte	0xc4
	.byte	0x39
	.string	"n"
	.byte	0x1
	.2byte	0x147
	.byte	0xe
	.4byte	0xc4
	.byte	0x39
	.string	"i"
	.byte	0x1
	.2byte	0x148
	.byte	0xe
	.4byte	0xc4
	.byte	0x39
	.string	"ch"
	.byte	0x1
	.2byte	0x149
	.byte	0xd
	.4byte	0xa7
	.byte	0x38
	.4byte	.LASF333
	.byte	0x1
	.2byte	0x14a
	.byte	0xd
	.4byte	0xa7
	.byte	0
	.byte	0x13
	.4byte	0xc4
	.4byte	0x1620
	.byte	0x14
	.4byte	0xa0
	.byte	0x3b
	.byte	0
	.byte	0x3a
	.4byte	.LASF342
	.byte	0x1
	.2byte	0x127
	.byte	0xd
	.byte	0x1
	.4byte	0x1654
	.byte	0x36
	.string	"src"
	.byte	0x1
	.2byte	0x127
	.byte	0x28
	.4byte	0x1654
	.byte	0x38
	.4byte	.LASF333
	.byte	0x1
	.2byte	0x129
	.byte	0xd
	.4byte	0xa7
	.byte	0x39
	.string	"i"
	.byte	0x1
	.2byte	0x12a
	.byte	0xe
	.4byte	0xc4
	.byte	0
	.byte	0x9
	.byte	0x4
	.4byte	0xd0
	.byte	0x3b
	.4byte	.LASF334
	.byte	0x1
	.2byte	0x11b
	.byte	0xd
	.4byte	.LFB41
	.4byte	.LFE41-.LFB41
	.byte	0x1
	.byte	0x9c
	.4byte	0x1695
	.byte	0x2d
	.4byte	.LASF331
	.byte	0x1
	.2byte	0x11d
	.byte	0xe
	.4byte	0xe1
	.4byte	.LLST4
	.byte	0x3c
	.4byte	.LVL7
	.4byte	0x1918
	.byte	0x3c
	.4byte	.LVL8
	.4byte	0x18d9
	.byte	0
	.byte	0x3b
	.4byte	.LASF335
	.byte	0x1
	.2byte	0x107
	.byte	0xd
	.4byte	.LFB40
	.4byte	.LFE40-.LFB40
	.byte	0x1
	.byte	0x9c
	.4byte	0x1717
	.byte	0x2e
	.4byte	0x1998
	.4byte	.LBB25
	.4byte	.LBE25-.LBB25
	.byte	0x1
	.2byte	0x109
	.byte	0x5
	.4byte	0x16cb
	.byte	0x2f
	.4byte	0x19a5
	.4byte	.LLST5
	.byte	0
	.byte	0x2e
	.4byte	0x1998
	.4byte	.LBB27
	.4byte	.LBE27-.LBB27
	.byte	0x1
	.2byte	0x10a
	.byte	0x5
	.4byte	0x16ea
	.byte	0x2f
	.4byte	0x19a5
	.4byte	.LLST6
	.byte	0
	.byte	0x3c
	.4byte	.LVL14
	.4byte	0x1882
	.byte	0x3d
	.4byte	.LVL15
	.4byte	0x1cee
	.4byte	0x170d
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x3
	.byte	0xa
	.2byte	0x200
	.byte	0x33
	.byte	0x1
	.byte	0x5b
	.byte	0x1
	.byte	0x30
	.byte	0
	.byte	0x3c
	.4byte	.LVL16
	.4byte	0x165a
	.byte	0
	.byte	0x3e
	.4byte	.LASF337
	.byte	0x1
	.byte	0xde
	.byte	0x11
	.4byte	0x12a
	.byte	0x1
	.4byte	0x1735
	.byte	0x3f
	.4byte	.LASF338
	.byte	0x1
	.byte	0xe0
	.byte	0xe
	.4byte	0xe1
	.byte	0
	.byte	0x40
	.4byte	.LASF360
	.byte	0x1
	.byte	0xcf
	.byte	0xd
	.byte	0x1
	.byte	0x3e
	.4byte	.LASF339
	.byte	0x1
	.byte	0xb7
	.byte	0x11
	.4byte	0xc4
	.byte	0x1
	.4byte	0x1766
	.byte	0x41
	.string	"raw"
	.byte	0x1
	.byte	0xb7
	.byte	0x24
	.4byte	0xc4
	.byte	0x42
	.string	"v"
	.byte	0x1
	.byte	0xb9
	.byte	0xd
	.4byte	0xd5
	.byte	0
	.byte	0x3e
	.4byte	.LASF340
	.byte	0x1
	.byte	0xa6
	.byte	0x10
	.4byte	0xa7
	.byte	0x1
	.4byte	0x178e
	.byte	0x42
	.string	"buf"
	.byte	0x1
	.byte	0xa8
	.byte	0xe
	.4byte	0x178e
	.byte	0x42
	.string	"i"
	.byte	0x1
	.byte	0xa9
	.byte	0xd
	.4byte	0xa7
	.byte	0
	.byte	0x13
	.4byte	0xc4
	.4byte	0x179e
	.byte	0x14
	.4byte	0xa0
	.byte	0x9
	.byte	0
	.byte	0x3e
	.4byte	.LASF341
	.byte	0x1
	.byte	0x78
	.byte	0x10
	.4byte	0xb8
	.byte	0x1
	.4byte	0x17f8
	.byte	0x41
	.string	"buf"
	.byte	0x1
	.byte	0x78
	.byte	0x2e
	.4byte	0x1654
	.byte	0x42
	.string	"t"
	.byte	0x1
	.byte	0x7a
	.byte	0xe
	.4byte	0xc4
	.byte	0x42
	.string	"p"
	.byte	0x1
	.byte	0x7b
	.byte	0xe
	.4byte	0xc4
	.byte	0x42
	.string	"i"
	.byte	0x1
	.byte	0x7c
	.byte	0xd
	.4byte	0xa7
	.byte	0x42
	.string	"j"
	.byte	0x1
	.byte	0x7d
	.byte	0xd
	.4byte	0xa7
	.byte	0x42
	.string	"a"
	.byte	0x1
	.byte	0x7e
	.byte	0xd
	.4byte	0xb8
	.byte	0x42
	.string	"b"
	.byte	0x1
	.byte	0x7f
	.byte	0xd
	.4byte	0xb8
	.byte	0
	.byte	0x43
	.4byte	.LASF343
	.byte	0x1
	.byte	0x63
	.byte	0xd
	.byte	0x1
	.4byte	0x183c
	.byte	0x41
	.string	"buf"
	.byte	0x1
	.byte	0x63
	.byte	0x20
	.4byte	0x3c5
	.byte	0x41
	.string	"n"
	.byte	0x1
	.byte	0x63
	.byte	0x2d
	.4byte	0xa7
	.byte	0x42
	.string	"i"
	.byte	0x1
	.byte	0x65
	.byte	0xd
	.4byte	0xa7
	.byte	0x42
	.string	"j"
	.byte	0x1
	.byte	0x66
	.byte	0xd
	.4byte	0xa7
	.byte	0x44
	.byte	0x42
	.string	"v"
	.byte	0x1
	.byte	0x6a
	.byte	0x12
	.4byte	0xc4
	.byte	0
	.byte	0
	.byte	0x3e
	.4byte	.LASF344
	.byte	0x1
	.byte	0x53
	.byte	0x10
	.4byte	0xa7
	.byte	0x1
	.4byte	0x185a
	.byte	0x41
	.string	"out"
	.byte	0x1
	.byte	0x53
	.byte	0x26
	.4byte	0x3c5
	.byte	0
	.byte	0x3e
	.4byte	.LASF345
	.byte	0x1
	.byte	0x45
	.byte	0x10
	.4byte	0xa7
	.byte	0x1
	.4byte	0x1882
	.byte	0x41
	.string	"bit"
	.byte	0x1
	.byte	0x45
	.byte	0x28
	.4byte	0xe1
	.byte	0x42
	.string	"n"
	.byte	0x1
	.byte	0x47
	.byte	0xe
	.4byte	0xc4
	.byte	0
	.byte	0x45
	.4byte	.LASF346
	.byte	0x1
	.byte	0x3a
	.byte	0xd
	.4byte	.LFB31
	.4byte	.LFE31-.LFB31
	.byte	0x1
	.byte	0x9c
	.4byte	0x18d9
	.byte	0x29
	.4byte	.LASF347
	.byte	0x1
	.byte	0x3c
	.byte	0x16
	.4byte	0x141b
	.byte	0x2
	.byte	0x91
	.byte	0x64
	.byte	0x3d
	.4byte	.LVL9
	.4byte	0x1cee
	.4byte	0x18bf
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x1
	.byte	0x34
	.byte	0x33
	.byte	0x1
	.byte	0x5b
	.byte	0x1
	.byte	0x31
	.byte	0
	.byte	0x32
	.4byte	.LVL10
	.4byte	0x1cfa
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x5
	.byte	0xc
	.4byte	0x40010800
	.byte	0x33
	.byte	0x1
	.byte	0x5b
	.byte	0x2
	.byte	0x91
	.byte	0x74
	.byte	0
	.byte	0
	.byte	0x45
	.4byte	.LASF348
	.byte	0x1
	.byte	0x32
	.byte	0xd
	.4byte	.LFB30
	.4byte	.LFE30-.LFB30
	.byte	0x1
	.byte	0x9c
	.4byte	0x1918
	.byte	0x46
	.4byte	.LASF331
	.byte	0x1
	.byte	0x32
	.byte	0x25
	.4byte	0xe1
	.byte	0x1
	.byte	0x5a
	.byte	0x47
	.4byte	0x19b2
	.4byte	.LBB23
	.4byte	.LBE23-.LBB23
	.byte	0x1
	.byte	0x36
	.byte	0x9
	.byte	0x2f
	.4byte	0x19bf
	.4byte	.LLST3
	.byte	0
	.byte	0
	.byte	0x48
	.4byte	.LASF361
	.byte	0x1
	.byte	0x29
	.byte	0x11
	.4byte	0xe1
	.4byte	.LFB29
	.4byte	.LFE29-.LFB29
	.byte	0x1
	.byte	0x9c
	.4byte	0x197a
	.byte	0x49
	.string	"on"
	.byte	0x1
	.byte	0x2b
	.byte	0xe
	.4byte	0xe1
	.4byte	.LLST0
	.byte	0x4a
	.4byte	0x197a
	.4byte	.LBB17
	.4byte	.Ldebug_ranges0+0
	.byte	0x1
	.byte	0x2d
	.byte	0xa
	.4byte	0x195f
	.byte	0x2f
	.4byte	0x198b
	.4byte	.LLST1
	.byte	0
	.byte	0x47
	.4byte	0x1998
	.4byte	.LBB20
	.4byte	.LBE20-.LBB20
	.byte	0x1
	.byte	0x2e
	.byte	0x5
	.byte	0x2f
	.4byte	0x19a5
	.4byte	.LLST2
	.byte	0
	.byte	0
	.byte	0x3e
	.4byte	.LASF349
	.byte	0x2
	.byte	0xc5
	.byte	0x3e
	.4byte	0xe1
	.byte	0x3
	.4byte	0x1998
	.byte	0x4b
	.4byte	.LASF350
	.byte	0x2
	.byte	0xc5
	.byte	0x5a
	.4byte	0x5bf
	.byte	0
	.byte	0x43
	.4byte	.LASF351
	.byte	0x2
	.byte	0xb6
	.byte	0x3a
	.byte	0x3
	.4byte	0x19b2
	.byte	0x4b
	.4byte	.LASF350
	.byte	0x2
	.byte	0xb6
	.byte	0x54
	.4byte	0x5bf
	.byte	0
	.byte	0x43
	.4byte	.LASF352
	.byte	0x2
	.byte	0xa8
	.byte	0x3a
	.byte	0x3
	.4byte	0x19cc
	.byte	0x4b
	.4byte	.LASF350
	.byte	0x2
	.byte	0xa8
	.byte	0x53
	.4byte	0x5bf
	.byte	0
	.byte	0x4c
	.4byte	0x1620
	.4byte	.LFB45
	.4byte	.LFE45-.LFB45
	.byte	0x1
	.byte	0x9c
	.4byte	0x19f5
	.byte	0x4d
	.4byte	0x162e
	.byte	0x1
	.byte	0x5a
	.byte	0x31
	.4byte	0x163b
	.byte	0x4e
	.4byte	0x1648
	.4byte	.LLST7
	.byte	0
	.byte	0x4c
	.4byte	0x158d
	.4byte	.LFB43
	.4byte	.LFE43-.LFB43
	.byte	0x1
	.byte	0x9c
	.4byte	0x1ae4
	.byte	0x2f
	.4byte	0x159f
	.4byte	.LLST8
	.byte	0x2f
	.4byte	0x15ac
	.4byte	.LLST9
	.byte	0x31
	.4byte	0x15b9
	.byte	0x31
	.4byte	0x15c6
	.byte	0x31
	.4byte	0x15d3
	.byte	0x31
	.4byte	0x15e0
	.byte	0x31
	.4byte	0x15eb
	.byte	0x31
	.4byte	0x15f6
	.byte	0x31
	.4byte	0x1602
	.byte	0x4f
	.4byte	0x158d
	.4byte	.Ldebug_ranges0+0x18
	.byte	0x2f
	.4byte	0x15ac
	.4byte	.LLST10
	.byte	0x2f
	.4byte	0x159f
	.4byte	.LLST11
	.byte	0x50
	.4byte	.Ldebug_ranges0+0x18
	.byte	0x4e
	.4byte	0x15b9
	.4byte	.LLST12
	.byte	0x51
	.4byte	0x15c6
	.byte	0x3
	.byte	0x91
	.byte	0xf8,0x7e
	.byte	0x4e
	.4byte	0x15d3
	.4byte	.LLST13
	.byte	0x4e
	.4byte	0x15e0
	.4byte	.LLST14
	.byte	0x4e
	.4byte	0x15eb
	.4byte	.LLST15
	.byte	0x4e
	.4byte	0x15f6
	.4byte	.LLST16
	.byte	0x31
	.4byte	0x1602
	.byte	0x52
	.4byte	0x173e
	.4byte	.LBB35
	.4byte	.Ldebug_ranges0+0x30
	.byte	0x1
	.2byte	0x168
	.byte	0x2e
	.4byte	0x1ac6
	.byte	0x2f
	.4byte	0x174f
	.4byte	.LLST17
	.byte	0x50
	.4byte	.Ldebug_ranges0+0x30
	.byte	0x4e
	.4byte	0x175b
	.4byte	.LLST18
	.byte	0
	.byte	0
	.byte	0x3c
	.4byte	.LVL22
	.4byte	0x1918
	.byte	0x3c
	.4byte	.LVL23
	.4byte	0x18d9
	.byte	0x3c
	.4byte	.LVL27
	.4byte	0x18d9
	.byte	0
	.byte	0
	.byte	0
	.byte	0x4c
	.4byte	0x1717
	.4byte	.LFB39
	.4byte	.LFE39-.LFB39
	.byte	0x1
	.byte	0x9c
	.4byte	0x1cee
	.byte	0x31
	.4byte	0x1728
	.byte	0x50
	.4byte	.Ldebug_ranges0+0x50
	.byte	0x4e
	.4byte	0x1728
	.4byte	.LLST19
	.byte	0x4a
	.4byte	0x1766
	.4byte	.LBB64
	.4byte	.Ldebug_ranges0+0x68
	.byte	0x1
	.byte	0xf7
	.byte	0x9
	.4byte	0x1c4a
	.byte	0x50
	.4byte	.Ldebug_ranges0+0x68
	.byte	0x51
	.4byte	0x1777
	.byte	0x2
	.byte	0x91
	.byte	0x5c
	.byte	0x4e
	.4byte	0x1783
	.4byte	.LLST20
	.byte	0x4a
	.4byte	0x183c
	.4byte	.LBB66
	.4byte	.Ldebug_ranges0+0x98
	.byte	0x1
	.byte	0xad
	.byte	0xd
	.4byte	0x1ba8
	.byte	0x53
	.4byte	0x184d
	.byte	0x54
	.4byte	0x185a
	.4byte	.LBB68
	.4byte	.LBE68-.LBB68
	.byte	0x1
	.byte	0x56
	.byte	0x9
	.4byte	0x1b7e
	.byte	0x2f
	.4byte	0x186b
	.4byte	.LLST21
	.byte	0x30
	.4byte	.LBB69
	.4byte	.LBE69-.LBB69
	.byte	0x4e
	.4byte	0x1877
	.4byte	.LLST22
	.byte	0
	.byte	0
	.byte	0x55
	.4byte	0x185a
	.4byte	.LBB70
	.4byte	.Ldebug_ranges0+0xc0
	.byte	0x1
	.byte	0x5b
	.byte	0x9
	.byte	0x2f
	.4byte	0x186b
	.4byte	.LLST23
	.byte	0x50
	.4byte	.Ldebug_ranges0+0xc0
	.byte	0x4e
	.4byte	0x1877
	.4byte	.LLST24
	.byte	0
	.byte	0
	.byte	0
	.byte	0x4a
	.4byte	0x179e
	.4byte	.LBB78
	.4byte	.Ldebug_ranges0+0xd8
	.byte	0x1
	.byte	0xb3
	.byte	0xd
	.4byte	0x1bfe
	.byte	0x53
	.4byte	0x17af
	.byte	0x50
	.4byte	.Ldebug_ranges0+0xd8
	.byte	0x4e
	.4byte	0x17bb
	.4byte	.LLST25
	.byte	0x4e
	.4byte	0x17c5
	.4byte	.LLST26
	.byte	0x4e
	.4byte	0x17cf
	.4byte	.LLST27
	.byte	0x4e
	.4byte	0x17d9
	.4byte	.LLST28
	.byte	0x4e
	.4byte	0x17e3
	.4byte	.LLST29
	.byte	0x4e
	.4byte	0x17ed
	.4byte	.LLST30
	.byte	0
	.byte	0
	.byte	0x47
	.4byte	0x17f8
	.4byte	.LBB83
	.4byte	.LBE83-.LBB83
	.byte	0x1
	.byte	0xb2
	.byte	0x5
	.byte	0x53
	.4byte	0x1811
	.byte	0x53
	.4byte	0x1805
	.byte	0x30
	.4byte	.LBB84
	.4byte	.LBE84-.LBB84
	.byte	0x4e
	.4byte	0x181b
	.4byte	.LLST31
	.byte	0x4e
	.4byte	0x1825
	.4byte	.LLST32
	.byte	0x4f
	.4byte	0x182f
	.4byte	.Ldebug_ranges0+0xf8
	.byte	0x4e
	.4byte	0x1830
	.4byte	.LLST33
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0x56
	.4byte	0x1735
	.4byte	.LBB96
	.4byte	.Ldebug_ranges0+0x110
	.byte	0x1
	.2byte	0x100
	.byte	0x5
	.byte	0x52
	.4byte	0x19b2
	.4byte	.LBB103
	.4byte	.Ldebug_ranges0+0x130
	.byte	0x1
	.2byte	0x102
	.byte	0x5
	.4byte	0x1c7a
	.byte	0x2f
	.4byte	0x19bf
	.4byte	.LLST34
	.byte	0
	.byte	0x57
	.4byte	.LVL39
	.4byte	0x1c9f
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x5
	.byte	0x3
	.4byte	.LANCHOR7
	.byte	0x33
	.byte	0x1
	.byte	0x5b
	.byte	0x5
	.byte	0x3
	.4byte	adc_irq_lock
	.byte	0x33
	.byte	0x1
	.byte	0x5c
	.byte	0x5
	.byte	0x3
	.4byte	adc_irq_unlock
	.byte	0
	.byte	0x3c
	.4byte	.LVL40
	.4byte	0x1882
	.byte	0x3d
	.4byte	.LVL41
	.4byte	0x1cee
	.4byte	0x1cc2
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x3
	.byte	0xa
	.2byte	0x200
	.byte	0x33
	.byte	0x1
	.byte	0x5b
	.byte	0x1
	.byte	0x31
	.byte	0
	.byte	0x3d
	.4byte	.LVL42
	.4byte	0x1d06
	.4byte	0x1cda
	.byte	0x33
	.byte	0x1
	.byte	0x5a
	.byte	0x1
	.byte	0x31
	.byte	0x33
	.byte	0x1
	.byte	0x5b
	.byte	0x1
	.byte	0x31
	.byte	0
	.byte	0x3c
	.4byte	.LVL51
	.4byte	0x1695
	.byte	0x3c
	.4byte	.LVL82
	.4byte	0x165a
	.byte	0
	.byte	0
	.byte	0x58
	.4byte	.LASF353
	.4byte	.LASF353
	.byte	0xf
	.byte	0xf1
	.byte	0xd
	.byte	0x58
	.4byte	.LASF354
	.4byte	.LASF354
	.byte	0xe
	.byte	0xa9
	.byte	0xa
	.byte	0x58
	.4byte	.LASF355
	.4byte	.LASF355
	.byte	0xf
	.byte	0xf0
	.byte	0xd
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.byte	0x1
	.byte	0x11
	.byte	0x1
	.byte	0x25
	.byte	0xe
	.byte	0x13
	.byte	0xb
	.byte	0x3
	.byte	0xe
	.byte	0x1b
	.byte	0xe
	.byte	0x55
	.byte	0x17
	.byte	0x11
	.byte	0x1
	.byte	0x10
	.byte	0x17
	.byte	0
	.byte	0
	.byte	0x2
	.byte	0x24
	.byte	0
	.byte	0xb
	.byte	0xb
	.byte	0x3e
	.byte	0xb
	.byte	0x3
	.byte	0xe
	.byte	0
	.byte	0
	.byte	0x3
	.byte	0x16
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x4
	.byte	0x24
	.byte	0
	.byte	0xb
	.byte	0xb
	.byte	0x3e
	.byte	0xb
	.byte	0x3
	.byte	0x8
	.byte	0
	.byte	0
	.byte	0x5
	.byte	0x35
	.byte	0
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x6
	.byte	0x26
	.byte	0
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x7
	.byte	0x4
	.byte	0x1
	.byte	0x3e
	.byte	0xb
	.byte	0xb
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x8
	.byte	0x28
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x1c
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x9
	.byte	0xf
	.byte	0
	.byte	0xb
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0xa
	.byte	0x15
	.byte	0x1
	.byte	0x27
	.byte	0x19
	.byte	0x49
	.byte	0x13
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0xb
	.byte	0x5
	.byte	0
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0xc
	.byte	0x13
	.byte	0x1
	.byte	0xb
	.byte	0xb
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0xd
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x38
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0xe
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x38
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0xf
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x3f
	.byte	0x19
	.byte	0x3c
	.byte	0x19
	.byte	0
	.byte	0
	.byte	0x10
	.byte	0x15
	.byte	0
	.byte	0x27
	.byte	0x19
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x11
	.byte	0x15
	.byte	0x1
	.byte	0x27
	.byte	0x19
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x12
	.byte	0x13
	.byte	0x1
	.byte	0xb
	.byte	0x5
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x13
	.byte	0x1
	.byte	0x1
	.byte	0x49
	.byte	0x13
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x14
	.byte	0x21
	.byte	0
	.byte	0x49
	.byte	0x13
	.byte	0x2f
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x15
	.byte	0x15
	.byte	0
	.byte	0x27
	.byte	0x19
	.byte	0
	.byte	0
	.byte	0x16
	.byte	0x4
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3e
	.byte	0xb
	.byte	0xb
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x17
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x38
	.byte	0x5
	.byte	0
	.byte	0
	.byte	0x18
	.byte	0x21
	.byte	0
	.byte	0x49
	.byte	0x13
	.byte	0x2f
	.byte	0x5
	.byte	0
	.byte	0
	.byte	0x19
	.byte	0x13
	.byte	0x1
	.byte	0xb
	.byte	0xb
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x1a
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x38
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x1b
	.byte	0x16
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x1c
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x38
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x1d
	.byte	0x13
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3c
	.byte	0x19
	.byte	0
	.byte	0
	.byte	0x1e
	.byte	0x17
	.byte	0x1
	.byte	0xb
	.byte	0xb
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x1f
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x20
	.byte	0xf
	.byte	0
	.byte	0xb
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x21
	.byte	0x13
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0xb
	.byte	0xb
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x22
	.byte	0x13
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0xb
	.byte	0x5
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x23
	.byte	0x13
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0xb
	.byte	0x5
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x24
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x38
	.byte	0x5
	.byte	0
	.byte	0
	.byte	0x25
	.byte	0x13
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0xb
	.byte	0xb
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x26
	.byte	0x17
	.byte	0x1
	.byte	0xb
	.byte	0xb
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x27
	.byte	0xd
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x28
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x3f
	.byte	0x19
	.byte	0x3c
	.byte	0x19
	.byte	0
	.byte	0
	.byte	0x29
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x2
	.byte	0x18
	.byte	0
	.byte	0
	.byte	0x2a
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x2
	.byte	0x18
	.byte	0
	.byte	0
	.byte	0x2b
	.byte	0x34
	.byte	0
	.byte	0x47
	.byte	0x13
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x2
	.byte	0x18
	.byte	0
	.byte	0
	.byte	0x2c
	.byte	0x2e
	.byte	0x1
	.byte	0x3f
	.byte	0x19
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x40
	.byte	0x18
	.byte	0x97,0x42
	.byte	0x19
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x2d
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x2
	.byte	0x17
	.byte	0
	.byte	0
	.byte	0x2e
	.byte	0x1d
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0x5
	.byte	0x57
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x2f
	.byte	0x5
	.byte	0
	.byte	0x31
	.byte	0x13
	.byte	0x2
	.byte	0x17
	.byte	0
	.byte	0
	.byte	0x30
	.byte	0xb
	.byte	0x1
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0
	.byte	0
	.byte	0x31
	.byte	0x34
	.byte	0
	.byte	0x31
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x32
	.byte	0x89,0x82,0x1
	.byte	0x1
	.byte	0x11
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x33
	.byte	0x8a,0x82,0x1
	.byte	0
	.byte	0x2
	.byte	0x18
	.byte	0x91,0x42
	.byte	0x18
	.byte	0
	.byte	0
	.byte	0x34
	.byte	0x1d
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0x5
	.byte	0x57
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x35
	.byte	0x2e
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x49
	.byte	0x13
	.byte	0x20
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x36
	.byte	0x5
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x37
	.byte	0x5
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x38
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x39
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x3a
	.byte	0x2e
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x20
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x3b
	.byte	0x2e
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0x5
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x40
	.byte	0x18
	.byte	0x97,0x42
	.byte	0x19
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x3c
	.byte	0x89,0x82,0x1
	.byte	0
	.byte	0x11
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x3d
	.byte	0x89,0x82,0x1
	.byte	0x1
	.byte	0x11
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x3e
	.byte	0x2e
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x49
	.byte	0x13
	.byte	0x20
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x3f
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x40
	.byte	0x2e
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x20
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x41
	.byte	0x5
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x42
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x43
	.byte	0x2e
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x20
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x44
	.byte	0xb
	.byte	0x1
	.byte	0
	.byte	0
	.byte	0x45
	.byte	0x2e
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x40
	.byte	0x18
	.byte	0x97,0x42
	.byte	0x19
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x46
	.byte	0x5
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x2
	.byte	0x18
	.byte	0
	.byte	0
	.byte	0x47
	.byte	0x1d
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0xb
	.byte	0x57
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x48
	.byte	0x2e
	.byte	0x1
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x27
	.byte	0x19
	.byte	0x49
	.byte	0x13
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x40
	.byte	0x18
	.byte	0x97,0x42
	.byte	0x19
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x49
	.byte	0x34
	.byte	0
	.byte	0x3
	.byte	0x8
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0x2
	.byte	0x17
	.byte	0
	.byte	0
	.byte	0x4a
	.byte	0x1d
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x52
	.byte	0x1
	.byte	0x55
	.byte	0x17
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0xb
	.byte	0x57
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x4b
	.byte	0x5
	.byte	0
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0x49
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x4c
	.byte	0x2e
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x40
	.byte	0x18
	.byte	0x97,0x42
	.byte	0x19
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x4d
	.byte	0x5
	.byte	0
	.byte	0x31
	.byte	0x13
	.byte	0x2
	.byte	0x18
	.byte	0
	.byte	0
	.byte	0x4e
	.byte	0x34
	.byte	0
	.byte	0x31
	.byte	0x13
	.byte	0x2
	.byte	0x17
	.byte	0
	.byte	0
	.byte	0x4f
	.byte	0xb
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x55
	.byte	0x17
	.byte	0
	.byte	0
	.byte	0x50
	.byte	0xb
	.byte	0x1
	.byte	0x55
	.byte	0x17
	.byte	0
	.byte	0
	.byte	0x51
	.byte	0x34
	.byte	0
	.byte	0x31
	.byte	0x13
	.byte	0x2
	.byte	0x18
	.byte	0
	.byte	0
	.byte	0x52
	.byte	0x1d
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x52
	.byte	0x1
	.byte	0x55
	.byte	0x17
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0x5
	.byte	0x57
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x53
	.byte	0x5
	.byte	0
	.byte	0x31
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x54
	.byte	0x1d
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x11
	.byte	0x1
	.byte	0x12
	.byte	0x6
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0xb
	.byte	0x57
	.byte	0xb
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x55
	.byte	0x1d
	.byte	0x1
	.byte	0x31
	.byte	0x13
	.byte	0x52
	.byte	0x1
	.byte	0x55
	.byte	0x17
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0xb
	.byte	0x57
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x56
	.byte	0x1d
	.byte	0
	.byte	0x31
	.byte	0x13
	.byte	0x52
	.byte	0x1
	.byte	0x55
	.byte	0x17
	.byte	0x58
	.byte	0xb
	.byte	0x59
	.byte	0x5
	.byte	0x57
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0x57
	.byte	0x89,0x82,0x1
	.byte	0x1
	.byte	0x11
	.byte	0x1
	.byte	0x1
	.byte	0x13
	.byte	0
	.byte	0
	.byte	0x58
	.byte	0x2e
	.byte	0
	.byte	0x3f
	.byte	0x19
	.byte	0x3c
	.byte	0x19
	.byte	0x6e
	.byte	0xe
	.byte	0x3
	.byte	0xe
	.byte	0x3a
	.byte	0xb
	.byte	0x3b
	.byte	0xb
	.byte	0x39
	.byte	0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST35:
	.4byte	.LVL91
	.4byte	.LVL94
	.2byte	0x1
	.byte	0x58
	.4byte	0
	.4byte	0
.LLST36:
	.4byte	.LVL92
	.4byte	.LVL93
	.2byte	0x6
	.byte	0x3
	.4byte	s_pp
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST37:
	.4byte	.LVL95
	.4byte	.LVL96
	.2byte	0x6
	.byte	0x3
	.4byte	s_pp+120
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL7
	.4byte	.LVL8-1
	.2byte	0x1
	.byte	0x5a
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL11
	.4byte	.LVL12
	.2byte	0x2
	.byte	0x4b
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LVL12
	.4byte	.LVL13
	.2byte	0x3
	.byte	0x8
	.byte	0x22
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL5
	.4byte	.LVL6
	.2byte	0x2
	.byte	0x4b
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST0:
	.4byte	.LVL1
	.4byte	.LVL3
	.2byte	0xc
	.byte	0x7a
	.byte	0
	.byte	0x40
	.byte	0x47
	.byte	0x24
	.byte	0x1a
	.byte	0x30
	.byte	0x2e
	.byte	0x8
	.byte	0xff
	.byte	0x1a
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL0
	.4byte	.LVL1
	.2byte	0x2
	.byte	0x4b
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL1
	.4byte	.LVL2
	.2byte	0x2
	.byte	0x4b
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL18
	.4byte	.LVL19
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST8:
	.4byte	.LVL20
	.4byte	.LVL22-1
	.2byte	0x1
	.byte	0x5a
	.4byte	.LVL22-1
	.4byte	.LVL37
	.2byte	0x1
	.byte	0x58
	.4byte	.LVL37
	.4byte	.LVL38
	.2byte	0x1
	.byte	0x5a
	.4byte	.LVL38
	.4byte	.LFE43
	.2byte	0x4
	.byte	0xf3
	.byte	0x1
	.byte	0x5a
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST9:
	.4byte	.LVL20
	.4byte	.LVL22-1
	.2byte	0x1
	.byte	0x5b
	.4byte	.LVL22-1
	.4byte	.LVL37
	.2byte	0x1
	.byte	0x59
	.4byte	.LVL37
	.4byte	.LFE43
	.2byte	0x1
	.byte	0x5b
	.4byte	0
	.4byte	0
.LLST10:
	.4byte	.LVL21
	.4byte	.LVL37
	.2byte	0x1
	.byte	0x59
	.4byte	0
	.4byte	0
.LLST11:
	.4byte	.LVL21
	.4byte	.LVL37
	.2byte	0x1
	.byte	0x58
	.4byte	0
	.4byte	0
.LLST12:
	.4byte	.LVL22
	.4byte	.LVL23-1
	.2byte	0x1
	.byte	0x5a
	.4byte	.LVL24
	.4byte	.LVL27-1
	.2byte	0x1
	.byte	0x5a
	.4byte	0
	.4byte	0
.LLST13:
	.4byte	.LVL27
	.4byte	.LVL37
	.2byte	0x2
	.byte	0x44
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST14:
	.4byte	.LVL27
	.4byte	.LVL28
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST15:
	.4byte	.LVL25
	.4byte	.LVL26
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST16:
	.4byte	.LVL28
	.4byte	.LVL29
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL29
	.4byte	.LVL34
	.2byte	0x5
	.byte	0x33
	.byte	0x7d
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	.LVL35
	.4byte	.LVL37
	.2byte	0x5
	.byte	0x33
	.byte	0x7d
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST17:
	.4byte	.LVL30
	.4byte	.LVL32
	.2byte	0x1
	.byte	0x5f
	.4byte	.LVL32
	.4byte	.LVL33
	.2byte	0x2
	.byte	0x7c
	.byte	0
	.4byte	.LVL35
	.4byte	.LVL36
	.2byte	0x1
	.byte	0x5f
	.4byte	.LVL36
	.4byte	.LVL37
	.2byte	0x2
	.byte	0x7c
	.byte	0
	.4byte	0
	.4byte	0
.LLST18:
	.4byte	.LVL31
	.4byte	.LVL33
	.2byte	0x1
	.byte	0x61
	.4byte	0
	.4byte	0
.LLST19:
	.4byte	.LVL43
	.4byte	.LVL85
	.2byte	0x4
	.byte	0xa
	.2byte	0x1ff
	.byte	0x9f
	.4byte	.LVL86
	.4byte	.LFE39
	.2byte	0x4
	.byte	0xa
	.2byte	0x1ff
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST20:
	.4byte	.LVL44
	.4byte	.LVL45
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST21:
	.4byte	.LVL46
	.4byte	.LVL50
	.2byte	0x2
	.byte	0x38
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST22:
	.4byte	.LVL46
	.4byte	.LVL47
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL47
	.4byte	.LVL48
	.2byte	0x7
	.byte	0xa
	.2byte	0x7d0
	.byte	0x7f
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	.LVL48
	.4byte	.LVL49
	.2byte	0x7
	.byte	0xa
	.2byte	0x7d1
	.byte	0x7f
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST23:
	.4byte	.LVL87
	.4byte	.LFE39
	.2byte	0x2
	.byte	0x34
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST24:
	.4byte	.LVL87
	.4byte	.LVL88
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL88
	.4byte	.LVL89
	.2byte	0x7
	.byte	0xa
	.2byte	0x7d0
	.byte	0x7f
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	.LVL89
	.4byte	.LVL90
	.2byte	0x7
	.byte	0xa
	.2byte	0x7d1
	.byte	0x7f
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST25:
	.4byte	.LVL52
	.4byte	.LVL53
	.2byte	0x1
	.byte	0x5c
	.4byte	.LVL55
	.4byte	.LVL56
	.2byte	0x1
	.byte	0x5c
	.4byte	.LVL56
	.4byte	.LVL63
	.2byte	0x1
	.byte	0x5d
	.4byte	.LVL72
	.4byte	.LVL73
	.2byte	0x1
	.byte	0x5c
	.4byte	.LVL73
	.4byte	.LVL79
	.2byte	0x1
	.byte	0x5d
	.4byte	0
	.4byte	0
.LLST26:
	.4byte	.LVL57
	.4byte	.LVL59
	.2byte	0x1
	.byte	0x5e
	.4byte	.LVL60
	.4byte	.LVL61
	.2byte	0x1
	.byte	0x5e
	.4byte	.LVL75
	.4byte	.LVL77
	.2byte	0x1
	.byte	0x5e
	.4byte	.LVL78
	.4byte	.LVL80
	.2byte	0x1
	.byte	0x5f
	.4byte	0
	.4byte	0
.LLST27:
	.4byte	.LVL52
	.4byte	.LVL54
	.2byte	0x1
	.byte	0x5f
	.4byte	.LVL55
	.4byte	.LVL58
	.2byte	0x1
	.byte	0x5f
	.4byte	.LVL61
	.4byte	.LVL62
	.2byte	0x1
	.byte	0x5f
	.4byte	.LVL72
	.4byte	.LVL76
	.2byte	0x1
	.byte	0x5f
	.4byte	.LVL76
	.4byte	.LVL78
	.2byte	0x1
	.byte	0x5a
	.4byte	0
	.4byte	0
.LLST28:
	.4byte	.LVL57
	.4byte	.LVL63
	.2byte	0x1
	.byte	0x5c
	.4byte	.LVL74
	.4byte	.LVL81
	.2byte	0x1
	.byte	0x5c
	.4byte	0
	.4byte	0
.LLST29:
	.4byte	.LVL78
	.4byte	.LVL79
	.2byte	0x7
	.byte	0xa
	.2byte	0x400
	.byte	0x7d
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST30:
	.4byte	.LVL78
	.4byte	.LVL80
	.2byte	0x7
	.byte	0xa
	.2byte	0x800
	.byte	0x7f
	.byte	0
	.byte	0x1c
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST31:
	.4byte	.LVL64
	.4byte	.LVL70
	.2byte	0x1
	.byte	0x5e
	.4byte	.LVL71
	.4byte	.LVL72
	.2byte	0x1
	.byte	0x5e
	.4byte	0
	.4byte	0
.LLST32:
	.4byte	.LVL65
	.4byte	.LVL66
	.2byte	0x1
	.byte	0x5e
	.4byte	.LVL66
	.4byte	.LVL67
	.2byte	0x1
	.byte	0x5f
	.4byte	.LVL68
	.4byte	.LVL69
	.2byte	0x1
	.byte	0x5f
	.4byte	0
	.4byte	0
.LLST33:
	.4byte	.LVL65
	.4byte	.LVL72
	.2byte	0x1
	.byte	0x60
	.4byte	0
	.4byte	0
.LLST34:
	.4byte	.LVL83
	.4byte	.LVL84
	.2byte	0x2
	.byte	0x4b
	.byte	0x9f
	.4byte	0
	.4byte	0
	.section	.debug_aranges,"",@progbits
	.4byte	0x5c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.LFB29
	.4byte	.LFE29-.LFB29
	.4byte	.LFB30
	.4byte	.LFE30-.LFB30
	.4byte	.LFB41
	.4byte	.LFE41-.LFB41
	.4byte	.LFB31
	.4byte	.LFE31-.LFB31
	.4byte	.LFB40
	.4byte	.LFE40-.LFB40
	.4byte	.LFB45
	.4byte	.LFE45-.LFB45
	.4byte	.LFB43
	.4byte	.LFE43-.LFB43
	.4byte	.LFB39
	.4byte	.LFE39-.LFB39
	.4byte	.LFB44
	.4byte	.LFE44-.LFB44
	.4byte	0
	.4byte	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.4byte	.LBB17
	.4byte	.LBE17
	.4byte	.LBB22
	.4byte	.LBE22
	.4byte	0
	.4byte	0
	.4byte	.LBB33
	.4byte	.LBE33
	.4byte	.LBB42
	.4byte	.LBE42
	.4byte	0
	.4byte	0
	.4byte	.LBB35
	.4byte	.LBE35
	.4byte	.LBB39
	.4byte	.LBE39
	.4byte	.LBB40
	.4byte	.LBE40
	.4byte	0
	.4byte	0
	.4byte	.LBB63
	.4byte	.LBE63
	.4byte	.LBB108
	.4byte	.LBE108
	.4byte	0
	.4byte	0
	.4byte	.LBB64
	.4byte	.LBE64
	.4byte	.LBB94
	.4byte	.LBE94
	.4byte	.LBB95
	.4byte	.LBE95
	.4byte	.LBB100
	.4byte	.LBE100
	.4byte	.LBB107
	.4byte	.LBE107
	.4byte	0
	.4byte	0
	.4byte	.LBB66
	.4byte	.LBE66
	.4byte	.LBB77
	.4byte	.LBE77
	.4byte	.LBB82
	.4byte	.LBE82
	.4byte	.LBB89
	.4byte	.LBE89
	.4byte	0
	.4byte	0
	.4byte	.LBB70
	.4byte	.LBE70
	.4byte	.LBB73
	.4byte	.LBE73
	.4byte	0
	.4byte	0
	.4byte	.LBB78
	.4byte	.LBE78
	.4byte	.LBB87
	.4byte	.LBE87
	.4byte	.LBB88
	.4byte	.LBE88
	.4byte	0
	.4byte	0
	.4byte	.LBB85
	.4byte	.LBE85
	.4byte	.LBB86
	.4byte	.LBE86
	.4byte	0
	.4byte	0
	.4byte	.LBB96
	.4byte	.LBE96
	.4byte	.LBB101
	.4byte	.LBE101
	.4byte	.LBB102
	.4byte	.LBE102
	.4byte	0
	.4byte	0
	.4byte	.LBB103
	.4byte	.LBE103
	.4byte	.LBB106
	.4byte	.LBE106
	.4byte	0
	.4byte	0
	.4byte	.LFB29
	.4byte	.LFE29
	.4byte	.LFB30
	.4byte	.LFE30
	.4byte	.LFB41
	.4byte	.LFE41
	.4byte	.LFB31
	.4byte	.LFE31
	.4byte	.LFB40
	.4byte	.LFE40
	.4byte	.LFB45
	.4byte	.LFE45
	.4byte	.LFB43
	.4byte	.LFE43
	.4byte	.LFB39
	.4byte	.LFE39
	.4byte	.LFB44
	.4byte	.LFE44
	.4byte	0
	.4byte	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF50:
	.string	"overrun"
.LASF327:
	.string	"s_q_n"
.LASF81:
	.string	"EXTI2_IRQn"
.LASF326:
	.string	"s_q_r"
.LASF352:
	.string	"NVIC_EnableIRQ"
.LASF312:
	.string	"GPIO_Mode_IPU"
.LASF325:
	.string	"s_q_w"
.LASF197:
	.string	"_ssize_t"
.LASF302:
	.string	"__locale_t"
.LASF143:
	.string	"SCTLR"
.LASF270:
	.string	"__sf"
.LASF347:
	.string	"gpio"
.LASF237:
	.string	"_read"
.LASF357:
	.string	"D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376\\fw_app\\03_Driver\\adc\\drv_adc.c"
.LASF238:
	.string	"_write"
.LASF17:
	.string	"int32_t"
.LASF146:
	.string	"STATR"
.LASF281:
	.string	"_asctime_buf"
.LASF264:
	.string	"_cvtlen"
.LASF88:
	.string	"DMA1_Channel5_IRQn"
.LASF300:
	.string	"_unused"
.LASF41:
	.string	"plat_hook_wdt_hang"
.LASF296:
	.string	"_wcsrtombs_state"
.LASF242:
	.string	"_nbuf"
.LASF212:
	.string	"__tm_sec"
.LASF289:
	.string	"_l64a_buf"
.LASF30:
	.string	"plat_hook_t"
.LASF64:
	.string	"drv_adc_driver_t"
.LASF77:
	.string	"FLASH_IRQn"
.LASF331:
	.string	"state"
.LASF246:
	.string	"_lock"
.LASF26:
	.string	"plat_hook_fn"
.LASF129:
	.string	"VTFADDR"
.LASF277:
	.string	"_mult"
.LASF101:
	.string	"TIM2_IRQn"
.LASF245:
	.string	"_data"
.LASF339:
	.string	"adc_apply"
.LASF65:
	.string	"drv_adc"
.LASF199:
	.string	"__wch"
.LASF2:
	.string	"__uint8_t"
.LASF176:
	.string	"INDR"
.LASF109:
	.string	"SPI2_IRQn"
.LASF234:
	.string	"_file"
.LASF33:
	.string	"result"
.LASF221:
	.string	"_on_exit_args"
.LASF319:
	.string	"GPIO_Speed"
.LASF42:
	.string	"plat_hook_evt_full"
.LASF49:
	.string	"ready"
.LASF25:
	.string	"result_t"
.LASF149:
	.string	"SAMPTR1"
.LASF150:
	.string	"SAMPTR2"
.LASF185:
	.string	"APB1PRSTR"
.LASF292:
	.string	"_mbrlen_state"
.LASF8:
	.string	"long int"
.LASF303:
	.string	"_impure_ptr"
.LASF261:
	.string	"_result_k"
.LASF231:
	.string	"_size"
.LASF309:
	.string	"GPIO_Mode_AIN"
.LASF282:
	.string	"_localtime_buf"
.LASF75:
	.string	"TAMPER_IRQn"
.LASF350:
	.string	"IRQn"
.LASF216:
	.string	"__tm_mon"
.LASF315:
	.string	"GPIO_Mode_AF_OD"
.LASF71:
	.string	"SysTick_IRQn"
.LASF346:
	.string	"adc_pins_idle"
.LASF112:
	.string	"USART3_IRQn"
.LASF279:
	.string	"_unused_rand"
.LASF0:
	.string	"signed char"
.LASF14:
	.string	"uint8_t"
.LASF119:
	.string	"DMA1_Channel8_IRQn"
.LASF321:
	.string	"GPIO_InitTypeDef"
.LASF202:
	.string	"__value"
.LASF62:
	.string	"pf_stop"
.LASF128:
	.string	"RESERVED0"
.LASF130:
	.string	"RESERVED1"
.LASF1:
	.string	"unsigned char"
.LASF134:
	.string	"RESERVED3"
.LASF136:
	.string	"RESERVED4"
.LASF138:
	.string	"RESERVED5"
.LASF140:
	.string	"RESERVED6"
.LASF142:
	.string	"RESERVED7"
.LASF316:
	.string	"GPIO_Mode_AF_PP"
.LASF173:
	.string	"DMA_TypeDef"
.LASF55:
	.string	"pf_pp_irq_half"
.LASF256:
	.string	"_unspecified_locale_info"
.LASF105:
	.string	"I2C1_ER_IRQn"
.LASF249:
	.string	"_reent"
.LASF304:
	.string	"_global_impure_ptr"
.LASF22:
	.string	"RES_NO_MEM"
.LASF68:
	.string	"Ecall_M_Mode_IRQn"
.LASF84:
	.string	"DMA1_Channel1_IRQn"
.LASF58:
	.string	"_Bool"
.LASF97:
	.string	"TIM1_BRK_IRQn"
.LASF175:
	.string	"CFGHR"
.LASF56:
	.string	"pf_pp_irq_full"
.LASF132:
	.string	"RESERVED2"
.LASF227:
	.string	"_ind"
.LASF323:
	.string	"s_cal"
.LASF92:
	.string	"USB_HP_CAN1_TX_IRQn"
.LASF29:
	.string	"char"
.LASF228:
	.string	"_fns"
.LASF240:
	.string	"_close"
.LASF313:
	.string	"GPIO_Mode_Out_OD"
.LASF5:
	.string	"__uint16_t"
.LASF360:
	.string	"adc_dma_arm"
.LASF144:
	.string	"PFIC_Type"
.LASF40:
	.string	"plat_hook_timebase_dead"
.LASF251:
	.string	"_stdin"
.LASF335:
	.string	"adc_stop"
.LASF329:
	.string	"flags"
.LASF307:
	.string	"GPIO_Speed_50MHz"
.LASF322:
	.string	"s_on"
.LASF60:
	.string	"pp_sw_driver"
.LASF115:
	.string	"USBWakeUp_IRQn"
.LASF103:
	.string	"TIM4_IRQn"
.LASF111:
	.string	"USART2_IRQn"
.LASF332:
	.string	"per_ch"
.LASF341:
	.string	"adc_cal_value"
.LASF324:
	.string	"s_pp"
.LASF104:
	.string	"I2C1_EV_IRQn"
.LASF187:
	.string	"APB2PCENR"
.LASF166:
	.string	"ADC_TypeDef"
.LASF124:
	.string	"RESERVED"
.LASF160:
	.string	"ISQR"
.LASF236:
	.string	"_cookie"
.LASF181:
	.string	"CTLR"
.LASF209:
	.string	"_wds"
.LASF301:
	.string	"__lock"
.LASF268:
	.string	"_sig_func"
.LASF28:
	.string	"fatal"
.LASF125:
	.string	"CFGR"
.LASF53:
	.string	"pp_t"
.LASF244:
	.string	"_offset"
.LASF20:
	.string	"RES_PARAM"
.LASF265:
	.string	"_cvtbuf"
.LASF100:
	.string	"TIM1_CC_IRQn"
.LASF80:
	.string	"EXTI1_IRQn"
.LASF174:
	.string	"CFGLR"
.LASF139:
	.string	"IACTR"
.LASF314:
	.string	"GPIO_Mode_Out_PP"
.LASF343:
	.string	"adc_sort"
.LASF126:
	.string	"GISR"
.LASF66:
	.string	"NonMaskableInt_IRQn"
.LASF358:
	.string	"D:\\desktop\\Protocol_Analyze\\Protocol_Analyze\\\310\355\274\376B\\obj"
.LASF328:
	.string	"s_q_drop"
.LASF73:
	.string	"WWDG_IRQn"
.LASF262:
	.string	"_p5s"
.LASF10:
	.string	"long unsigned int"
.LASF353:
	.string	"RCC_APB2PeriphClockCmd"
.LASF188:
	.string	"APB1PCENR"
.LASF167:
	.string	"CNTR"
.LASF123:
	.string	"ITHRESDR"
.LASF87:
	.string	"DMA1_Channel4_IRQn"
.LASF258:
	.string	"__sdidinit"
.LASF248:
	.string	"_flags2"
.LASF141:
	.string	"IPRIOR"
.LASF145:
	.string	"SystemCoreClock"
.LASF95:
	.string	"CAN1_SCE_IRQn"
.LASF120:
	.string	"IRQn_Type"
.LASF31:
	.string	"arg0"
.LASF32:
	.string	"arg1"
.LASF131:
	.string	"IENR"
.LASF250:
	.string	"_errno"
.LASF169:
	.string	"MADDR"
.LASF183:
	.string	"INTR"
.LASF290:
	.string	"_signal_buf"
.LASF179:
	.string	"LCKR"
.LASF359:
	.string	"DMA1_Channel1_IRQHandler"
.LASF107:
	.string	"I2C2_ER_IRQn"
.LASF210:
	.string	"_Bigint"
.LASF177:
	.string	"OUTDR"
.LASF38:
	.string	"plat_hook_sched_reenter"
.LASF207:
	.string	"_maxwds"
.LASF191:
	.string	"AHBRSTR"
.LASF259:
	.string	"__cleanup"
.LASF48:
	.string	"pp_unlock_fn"
.LASF267:
	.string	"_atexit0"
.LASF61:
	.string	"pf_start"
.LASF9:
	.string	"__uint32_t"
.LASF255:
	.string	"_emergency"
.LASF11:
	.string	"long long int"
.LASF273:
	.string	"_niobs"
.LASF108:
	.string	"SPI1_IRQn"
.LASF269:
	.string	"__sglue"
.LASF337:
	.string	"adc_start"
.LASF43:
	.string	"plat_hook_log_flash"
.LASF299:
	.string	"_nmalloc"
.LASF283:
	.string	"_gamma_signgam"
.LASF178:
	.string	"BSHR"
.LASF263:
	.string	"_freelist"
.LASF274:
	.string	"_iobs"
.LASF272:
	.string	"_glue"
.LASF208:
	.string	"_sign"
.LASF114:
	.string	"RTCAlarm_IRQn"
.LASF342:
	.string	"adc_q_push"
.LASF118:
	.string	"UART4_IRQn"
.LASF54:
	.string	"pf_pp_init"
.LASF348:
	.string	"adc_irq_unlock"
.LASF39:
	.string	"plat_hook_sched_overrun"
.LASF156:
	.string	"WDLTR"
.LASF186:
	.string	"AHBPCENR"
.LASF13:
	.string	"unsigned int"
.LASF182:
	.string	"CFGR0"
.LASF297:
	.string	"_h_errno"
.LASF192:
	.string	"CFGR2"
.LASF106:
	.string	"I2C2_EV_IRQn"
.LASF19:
	.string	"RES_OK"
.LASF295:
	.string	"_wcrtomb_state"
.LASF215:
	.string	"__tm_mday"
.LASF266:
	.string	"_new"
.LASF241:
	.string	"_ubuf"
.LASF90:
	.string	"DMA1_Channel7_IRQn"
.LASF253:
	.string	"_stderr"
.LASF67:
	.string	"EXC_IRQn"
.LASF247:
	.string	"_mbstate"
.LASF284:
	.string	"_rand_next"
.LASF233:
	.string	"_flags"
.LASF226:
	.string	"_atexit"
.LASF211:
	.string	"__tm"
.LASF271:
	.string	"__FILE"
.LASF201:
	.string	"__count"
.LASF218:
	.string	"__tm_wday"
.LASF121:
	.string	"DISABLE"
.LASF219:
	.string	"__tm_yday"
.LASF147:
	.string	"CTLR1"
.LASF148:
	.string	"CTLR2"
.LASF338:
	.string	"samp"
.LASF349:
	.string	"NVIC_GetStatusIRQ"
.LASF276:
	.string	"_seed"
.LASF239:
	.string	"_seek"
.LASF35:
	.string	"g_plat_last_fault"
.LASF196:
	.string	"_fpos_t"
.LASF200:
	.string	"__wchb"
.LASF99:
	.string	"TIM1_TRG_COM_IRQn"
.LASF170:
	.string	"DMA_Channel_TypeDef"
.LASF354:
	.string	"GPIO_Init"
.LASF287:
	.string	"_mbtowc_state"
.LASF127:
	.string	"VTFIDR"
.LASF113:
	.string	"EXTI15_10_IRQn"
.LASF12:
	.string	"long long unsigned int"
.LASF117:
	.string	"USBFSWakeUp_IRQn"
.LASF16:
	.string	"uint16_t"
.LASF288:
	.string	"_wctomb_state"
.LASF223:
	.string	"_dso_handle"
.LASF351:
	.string	"NVIC_DisableIRQ"
.LASF275:
	.string	"_rand48"
.LASF345:
	.string	"adc_wait_clear"
.LASF102:
	.string	"TIM3_IRQn"
.LASF252:
	.string	"_stdout"
.LASF110:
	.string	"USART1_IRQn"
.LASF23:
	.string	"RES_STATE"
.LASF243:
	.string	"_blksize"
.LASF230:
	.string	"_base"
.LASF320:
	.string	"GPIO_Mode"
.LASF280:
	.string	"_strtok_last"
.LASF91:
	.string	"ADC_IRQn"
.LASF69:
	.string	"Ecall_U_Mode_IRQn"
.LASF293:
	.string	"_mbrtowc_state"
.LASF44:
	.string	"plat_hook_task_fail"
.LASF204:
	.string	"_flock_t"
.LASF344:
	.string	"adc_cal_one"
.LASF155:
	.string	"WDHTR"
.LASF172:
	.string	"INTFCR"
.LASF317:
	.string	"GPIOMode_TypeDef"
.LASF3:
	.string	"__int16_t"
.LASF203:
	.string	"_mbstate_t"
.LASF225:
	.string	"_is_cxa"
.LASF285:
	.string	"_r48"
.LASF74:
	.string	"PVD_IRQn"
.LASF198:
	.string	"wint_t"
.LASF63:
	.string	"pf_take"
.LASF133:
	.string	"IRER"
.LASF206:
	.string	"_next"
.LASF37:
	.string	"plat_hook_sched_bad_table"
.LASF79:
	.string	"EXTI0_IRQn"
.LASF306:
	.string	"GPIO_Speed_2MHz"
.LASF137:
	.string	"IPRR"
.LASF308:
	.string	"GPIOSpeed_TypeDef"
.LASF318:
	.string	"GPIO_Pin"
.LASF93:
	.string	"USB_LP_CAN1_RX0_IRQn"
.LASF27:
	.string	"name"
.LASF86:
	.string	"DMA1_Channel3_IRQn"
.LASF193:
	.string	"RCC_TypeDef"
.LASF286:
	.string	"_mblen_state"
.LASF4:
	.string	"short int"
.LASF135:
	.string	"IPSR"
.LASF78:
	.string	"RCC_IRQn"
.LASF310:
	.string	"GPIO_Mode_IN_FLOATING"
.LASF157:
	.string	"RSQR1"
.LASF158:
	.string	"RSQR2"
.LASF159:
	.string	"RSQR3"
.LASF15:
	.string	"int16_t"
.LASF21:
	.string	"RES_BUSY"
.LASF224:
	.string	"_fntypes"
.LASF336:
	.string	"adc_take"
.LASF122:
	.string	"ENABLE"
.LASF217:
	.string	"__tm_year"
.LASF334:
	.string	"adc_q_clear"
.LASF70:
	.string	"Break_Point_IRQn"
.LASF52:
	.string	"unlock"
.LASF161:
	.string	"IDATAR1"
.LASF162:
	.string	"IDATAR2"
.LASF163:
	.string	"IDATAR3"
.LASF164:
	.string	"IDATAR4"
.LASF180:
	.string	"GPIO_TypeDef"
.LASF235:
	.string	"_lbfsize"
.LASF189:
	.string	"BDCTLR"
.LASF254:
	.string	"_inc"
.LASF85:
	.string	"DMA1_Channel2_IRQn"
.LASF229:
	.string	"__sbuf"
.LASF47:
	.string	"pp_lock_fn"
.LASF194:
	.string	"_LOCK_T"
.LASF291:
	.string	"_getdate_err"
.LASF46:
	.string	"pp_sample_t"
.LASF298:
	.string	"_nextf"
.LASF257:
	.string	"_locale"
.LASF205:
	.string	"__ULong"
.LASF72:
	.string	"Software_IRQn"
.LASF18:
	.string	"uint32_t"
.LASF190:
	.string	"RSTSCKR"
.LASF98:
	.string	"TIM1_UP_IRQn"
.LASF260:
	.string	"_result"
.LASF94:
	.string	"CAN1_RX1_IRQn"
.LASF340:
	.string	"adc_calibrate"
.LASF36:
	.string	"g_plat_reset_reason"
.LASF330:
	.string	"rounds"
.LASF356:
	.string	"GNU C99 8.2.0 -march=rv32imacxw -mabi=ilp32 -msmall-data-limit= 8 -msave-restore -g -Os -std=gnu99 -fsigned-char -ffunction-sections -fdata-sections -fno-common"
.LASF96:
	.string	"EXTI9_5_IRQn"
.LASF195:
	.string	"_off_t"
.LASF171:
	.string	"INTFR"
.LASF278:
	.string	"_add"
.LASF305:
	.string	"GPIO_Speed_10MHz"
.LASF361:
	.string	"adc_irq_lock"
.LASF83:
	.string	"EXTI4_IRQn"
.LASF6:
	.string	"short unsigned int"
.LASF214:
	.string	"__tm_hour"
.LASF232:
	.string	"__sFILE"
.LASF333:
	.string	"slot"
.LASF82:
	.string	"EXTI3_IRQn"
.LASF294:
	.string	"_mbsrtowcs_state"
.LASF57:
	.string	"pf_pp_get"
.LASF7:
	.string	"__int32_t"
.LASF45:
	.string	"plat_hook_flash"
.LASF165:
	.string	"RDATAR"
.LASF222:
	.string	"_fnargs"
.LASF220:
	.string	"__tm_isdst"
.LASF89:
	.string	"DMA1_Channel6_IRQn"
.LASF24:
	.string	"RES_OVERRUN"
.LASF168:
	.string	"PADDR"
.LASF213:
	.string	"__tm_min"
.LASF151:
	.string	"IOFR1"
.LASF152:
	.string	"IOFR2"
.LASF153:
	.string	"IOFR3"
.LASF154:
	.string	"IOFR4"
.LASF355:
	.string	"RCC_AHBPeriphClockCmd"
.LASF51:
	.string	"lock"
.LASF76:
	.string	"RTC_IRQn"
.LASF184:
	.string	"APB2PRSTR"
.LASF59:
	.string	"pp_driver_interface_t"
.LASF116:
	.string	"USBFS_IRQn"
.LASF34:
	.string	"plat_fault_info_t"
.LASF311:
	.string	"GPIO_Mode_IPD"
	.ident	"GCC: (xPack GNU RISC-V Embedded GCC, 32-bit) 8.2.0"
