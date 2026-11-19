	.file	"drv_adc.c"
	.option nopic
	.text
	.section	.text.adc_irq_lock,"ax",@progbits
	.align	1
	.type	adc_irq_lock, @function
adc_irq_lock:
	li	a5,-536813568
	lw	a0,0(a5)
	li	a4,134217728
	sw	a4,384(a5)
	srli	a0,a0,27
	andi	a0,a0,1
	ret
	.size	adc_irq_lock, .-adc_irq_lock
	.section	.text.adc_irq_unlock,"ax",@progbits
	.align	1
	.type	adc_irq_unlock, @function
adc_irq_unlock:
	beqz	a0,.L2
	li	a5,-536813568
	li	a4,134217728
	sw	a4,256(a5)
.L2:
	ret
	.size	adc_irq_unlock, .-adc_irq_unlock
	.section	.text.adc_q_clear,"ax",@progbits
	.align	1
	.type	adc_q_clear, @function
adc_q_clear:
	call	t0,__riscv_save_0
	call	adc_irq_lock
	lui	a5,%hi(.LANCHOR0)
	sb	zero,%lo(.LANCHOR0)(a5)
	lui	a5,%hi(.LANCHOR1)
	sb	zero,%lo(.LANCHOR1)(a5)
	lui	a5,%hi(.LANCHOR2)
	sb	zero,%lo(.LANCHOR2)(a5)
	call	adc_irq_unlock
	tail	__riscv_restore_0
	.size	adc_q_clear, .-adc_q_clear
	.section	.text.adc_pins_idle,"ax",@progbits
	.align	1
	.type	adc_pins_idle, @function
adc_pins_idle:
	call	t0,__riscv_save_0
	addi	sp,sp,-16
	li	a1,1
	li	a0,4
	call	RCC_APB2PeriphClockCmd
	li	a5,7
	li	a0,1073811456
	sh	a5,4(sp)
	addi	a1,sp,4
	li	a5,3
	addi	a0,a0,-2048
	sw	a5,8(sp)
	sw	zero,12(sp)
	call	GPIO_Init
	addi	sp,sp,16
	tail	__riscv_restore_0
	.size	adc_pins_idle, .-adc_pins_idle
	.section	.text.adc_stop,"ax",@progbits
	.align	1
	.type	adc_stop, @function
adc_stop:
	call	t0,__riscv_save_0
	li	a5,-536813568
	li	a4,134217728
	sw	a4,384(a5)
	li	a4,4
	sw	a4,388(a5)
	li	a5,1073876992
	lw	a5,20(a5)
	andi	a5,a5,1
	beqz	a5,.L12
	li	a5,1073872896
	lw	a4,8(a5)
	andi	a4,a4,-2
	sw	a4,8(a5)
	sw	zero,12(a5)
	li	a4,15
	sw	a4,4(a5)
.L12:
	li	a5,1073876992
	lw	a5,24(a5)
	andi	a5,a5,512
	beqz	a5,.L13
	li	a5,1073815552
	lw	a4,1032(a5)
	li	a3,-5242880
	addi	a3,a3,-260
	and	a4,a4,a3
	sw	a4,1032(a5)
.L13:
	call	adc_pins_idle
	li	a1,0
	li	a0,512
	call	RCC_APB2PeriphClockCmd
	lui	a5,%hi(.LANCHOR3)
	sb	zero,%lo(.LANCHOR3)(a5)
	call	adc_q_clear
	tail	__riscv_restore_0
	.size	adc_stop, .-adc_stop
	.section	.text.adc_q_push.part.0,"ax",@progbits
	.align	1
	.type	adc_q_push.part.0, @function
adc_q_push.part.0:
	lui	a4,%hi(.LANCHOR2)
	addi	a5,a4,%lo(.LANCHOR2)
	lbu	a5,0(a5)
	li	a3,9
	addi	a4,a4,%lo(.LANCHOR2)
	bgeu	a3,a5,.L23
	lui	a3,%hi(.LANCHOR1)
	addi	a3,a3,%lo(.LANCHOR1)
	lbu	a5,0(a3)
	li	a2,10
	addi	a5,a5,1
	remu	a5,a5,a2
	sb	a5,0(a3)
	lbu	a5,0(a4)
	li	a3,-1
	addi	a5,a5,-1
	andi	a5,a5,0xff
	sb	a5,0(a4)
	lui	a5,%hi(.LANCHOR4)
	addi	a5,a5,%lo(.LANCHOR4)
	lw	a2,0(a5)
	beq	a2,a3,.L23
	lw	a3,0(a5)
	addi	a3,a3,1
	sw	a3,0(a5)
.L23:
	lui	a2,%hi(.LANCHOR0)
	addi	a5,a2,%lo(.LANCHOR0)
	lbu	a5,0(a5)
	li	a7,120
	lui	a6,%hi(.LANCHOR5)
	andi	a5,a5,0xff
	mul	a7,a5,a7
	li	a3,0
	addi	a2,a2,%lo(.LANCHOR0)
	addi	a6,a6,%lo(.LANCHOR5)
	li	t1,120
.L25:
	add	a1,a0,a3
	lhu	t3,0(a1)
	add	a1,a3,a7
	add	a1,a6,a1
	sh	t3,0(a1)
	addi	a3,a3,2
	bne	a3,t1,.L25
	addi	a5,a5,1
	li	a3,10
	remu	a5,a5,a3
	sb	a5,0(a2)
	lbu	a5,0(a4)
	addi	a5,a5,1
	andi	a5,a5,0xff
	sb	a5,0(a4)
	ret
	.size	adc_q_push.part.0, .-adc_q_push.part.0
	.section	.text.adc_take,"ax",@progbits
	.align	1
	.type	adc_take, @function
adc_take:
	beqz	a1,.L28
	sh	zero,0(a1)
.L28:
	li	a5,0
	beqz	a0,.L55
	call	t0,__riscv_save_2
	addi	sp,sp,-128
	mv	s1,a1
	mv	s0,a0
	call	adc_irq_lock
	lui	a5,%hi(.LANCHOR3)
	lbu	a5,%lo(.LANCHOR3)(a5)
	andi	a5,a5,0xff
	beqz	a5,.L30
	lui	a4,%hi(.LANCHOR2)
	addi	a5,a4,%lo(.LANCHOR2)
	lbu	a5,0(a5)
	addi	a4,a4,%lo(.LANCHOR2)
	andi	a5,a5,0xff
	bnez	a5,.L31
.L30:
	call	adc_irq_unlock
	li	a5,0
.L29:
	mv	a0,a5
	addi	sp,sp,128
	tail	__riscv_restore_2
.L31:
	lui	a2,%hi(.LANCHOR1)
	addi	a5,a2,%lo(.LANCHOR1)
	lbu	a5,0(a5)
	li	a7,120
	lui	a6,%hi(.LANCHOR5)
	andi	a5,a5,0xff
	mul	a7,a5,a7
	li	a3,0
	addi	a2,a2,%lo(.LANCHOR1)
	addi	a6,a6,%lo(.LANCHOR5)
	li	t1,120
.L33:
	addi	a1,sp,8
	add	t3,a1,a3
	add	a1,a3,a7
	add	a1,a6,a1
	lhu	a1,0(a1)
	addi	a3,a3,2
	sh	a1,0(t3)
	bne	a3,t1,.L33
	li	a3,10
	addi	a5,a5,1
	remu	a5,a5,a3
	sb	a5,0(a2)
	lbu	a5,0(a4)
	addi	a5,a5,-1
	andi	a5,a5,0xff
	sb	a5,0(a4)
	call	adc_irq_unlock
	lui	a5,%hi(.LANCHOR6)
	lh	t3,%lo(.LANCHOR6)(a5)
	li	a6,4096
	addi	a4,sp,8
	mv	a0,s0
	addi	t4,s0,40
	addi	t5,a6,-2
	addi	t1,a6,-1
.L34:
	mv	a1,a0
	mv	a2,a4
	li	a3,3
.L36:
	lhu	a5,0(a2)
	beqz	a5,.L35
	bgtu	a5,t5,.L39
	add	a7,a5,t3
	li	a5,0
	bltz	a7,.L35
	mv	a5,t1
	bge	a7,a6,.L35
	slli	a5,a7,16
	srli	a5,a5,16
.L35:
	addi	a3,a3,-1
	sh	a5,0(a1)
	andi	a3,a3,0xff
	addi	a2,a2,2
	addi	a1,a1,40
	bnez	a3,.L36
	addi	a0,a0,2
	addi	a4,a4,6
	bne	a0,t4,.L34
	li	a5,1
	beqz	s1,.L29
	li	a5,20
	sh	a5,0(s1)
	li	a5,1
	j	.L29
.L39:
	mv	a5,t1
	j	.L35
.L55:
	mv	a0,a5
	ret
	.size	adc_take, .-adc_take
	.section	.text.adc_start,"ax",@progbits
	.align	1
	.type	adc_start, @function
adc_start:
	call	t0,__riscv_save_3
	lui	s0,%hi(.LANCHOR3)
	addi	a5,s0,%lo(.LANCHOR3)
	lbu	a5,0(a5)
	addi	sp,sp,-32
	li	a0,0
	andi	a5,a5,0xff
	bnez	a5,.L58
	lui	a5,%hi(pp_sw_driver)
	lw	a5,%lo(pp_sw_driver)(a5)
	lui	s1,%hi(.LANCHOR7)
	lui	a2,%hi(adc_irq_unlock)
	lui	a1,%hi(adc_irq_lock)
	addi	a2,a2,%lo(adc_irq_unlock)
	addi	a1,a1,%lo(adc_irq_lock)
	addi	a0,s1,%lo(.LANCHOR7)
	jalr	a5
	call	adc_pins_idle
	li	a4,1073876992
	lw	a5,4(a4)
	li	a3,49152
	li	a1,1
	or	a5,a5,a3
	sw	a5,4(a4)
	li	a0,512
	call	RCC_APB2PeriphClockCmd
	li	a1,1
	li	a0,1
	call	RCC_AHBPeriphClockCmd
	li	a5,1073815552
	lw	a4,1032(a5)
	addi	a3,sp,12
	addi	s0,s0,%lo(.LANCHOR3)
	andi	a4,a4,-2
	sw	a4,1032(a5)
	li	a4,256
	sw	a4,1028(a5)
	li	a4,917504
	addi	a4,a4,2
	sw	a4,1032(a5)
	li	a4,2097152
	sw	a4,1068(a5)
	li	a4,4096
	sw	zero,1072(a5)
	addi	a4,a4,-2016
	sw	a4,1076(a5)
	li	a4,511
	sw	a4,1040(a5)
	sw	zero,1036(a5)
	lw	a4,1032(a5)
	addi	a7,sp,32
	mv	a1,a3
	ori	a4,a4,1
	sw	a4,1032(a5)
	li	a0,1073815552
	mv	a4,a3
.L76:
	lw	a5,1032(a0)
	ori	a5,a5,8
	sw	a5,1032(a0)
	li	a5,2000
.L61:
	addi	a2,a0,1024
	lw	a6,8(a2)
	andi	a6,a6,8
	beqz	a6,.L60
	addi	a5,a5,-1
	slli	a5,a5,16
	srli	a5,a5,16
	bnez	a5,.L61
.L62:
	call	adc_stop
	li	a0,5
	j	.L58
.L71:
	add	a2,a4,a2
	addi	a5,a5,1
	slli	a2,a2,16
	andi	a5,a5,0xff
	srli	a2,a2,16
	addi	a3,a3,2
	bne	a5,a6,.L69
.L70:
	rem	a4,a2,a5
	slli	a4,a4,1
	div	a3,a2,a5
	divu	a4,a4,a5
	add	a4,a4,a3
	slli	a3,a4,16
	srli	a3,a3,16
	j	.L72
.L75:
	slli	a0,a5,1
	add	a0,a1,a0
	lhu	a0,4(a0)
	addi	a5,a5,1
	add	a4,a4,a0
	slli	a4,a4,16
	srli	a4,a4,16
	j	.L74
.L80:
	li	a5,4096
	addi	a5,a5,-2048
	j	.L73
.L63:
	lw	a5,76(a6)
	addi	a4,a4,2
	sh	a5,-2(a4)
	bne	a7,a4,.L76
	addi	a2,sp,14
	li	a4,1
	li	t1,10
.L68:
	lhu	a6,0(a2)
	mv	a0,a2
	mv	a5,a4
.L65:
	lhu	a7,-2(a0)
	bgeu	a6,a7,.L66
	addi	a5,a5,-1
	sh	a7,0(a0)
	andi	a5,a5,0xff
	addi	a0,a0,-2
	bnez	a5,.L65
.L66:
	slli	a5,a5,1
	addi	a0,sp,32
	add	a5,a0,a5
	addi	a4,a4,1
	sh	a6,-20(a5)
	andi	a4,a4,0xff
	addi	a2,a2,2
	bne	a4,t1,.L68
	li	a5,0
	li	a2,0
	li	a0,1536
	li	a6,6
.L69:
	lhu	a4,4(a3)
	bleu	a4,a0,.L71
	li	a3,1024
	bnez	a5,.L70
.L72:
	li	a4,6
	sub	a4,a4,a5
	andi	a2,a4,0xff
	beqz	a2,.L80
	li	a4,0
	li	a6,5
.L74:
	andi	a0,a5,0xff
	bleu	a0,a6,.L75
	rem	a5,a4,a2
	slli	a5,a5,1
	divu	a5,a5,a2
	div	a4,a4,a2
	add	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
.L73:
	li	s2,4096
	li	a4,1024
	sub	a4,a4,a3
	addi	a3,s2,-2048
	sub	a5,a3,a5
	slli	a4,a4,16
	slli	a5,a5,16
	srai	a4,a4,16
	srai	a5,a5,16
	li	a3,2
	add	a5,a4,a5
	div	a4,a5,a3
	addi	s1,s1,%lo(.LANCHOR7)
	addi	s2,s2,1446
	rem	a5,a5,a3
	add	a5,a4,a5
	lui	a4,%hi(.LANCHOR6)
	sh	a5,%lo(.LANCHOR6)(a4)
	call	adc_q_clear
	lui	a5,%hi(.LANCHOR4)
	sw	zero,%lo(.LANCHOR4)(a5)
	li	a5,1
	sb	a5,0(s0)
	li	a4,1073872896
	lw	a5,8(a4)
	li	a3,15
	li	a0,0
	andi	a5,a5,-2
	sw	a5,8(a4)
	li	a5,1073815552
	addi	a2,a5,1100
	sw	a2,16(a4)
	sw	s1,20(a4)
	li	a2,120
	sw	a2,12(a4)
	sw	s2,8(a4)
	sw	a3,4(a4)
	lw	a3,8(a4)
	ori	a3,a3,1
	sw	a3,8(a4)
	lw	a4,1032(a5)
	li	a3,134217728
	ori	a4,a4,256
	sw	a4,1032(a5)
	li	a4,-536813568
	sw	a3,256(a4)
	lw	a4,1032(a5)
	li	a3,5242880
	or	a4,a4,a3
	sw	a4,1032(a5)
.L58:
	addi	sp,sp,32
	tail	__riscv_restore_3
.L60:
	lw	a5,8(a2)
	ori	a5,a5,4
	sw	a5,8(a2)
	li	a5,2000
.L64:
	addi	a6,a0,1024
	lw	a2,8(a6)
	andi	a2,a2,4
	beqz	a2,.L63
	addi	a5,a5,-1
	slli	a5,a5,16
	srli	a5,a5,16
	bnez	a5,.L64
	j	.L62
	.size	adc_start, .-adc_start
	.section	.text.DMA1_Channel1_IRQHandler,"ax",@progbits
	.align	1
	.globl	DMA1_Channel1_IRQHandler
	.type	DMA1_Channel1_IRQHandler, @function
DMA1_Channel1_IRQHandler:
	addi	sp,sp,-16
	li	a5,1073872896
	sw	s0,12(sp)
	lw	s0,0(a5)
	andi	a4,s0,4
	beqz	a4,.L93
	li	a4,4
	sw	a4,4(a5)
	lui	a5,%hi(.LANCHOR3)
	lbu	a5,%lo(.LANCHOR3)(a5)
	andi	a5,a5,0xff
	beqz	a5,.L93
	lui	a0,%hi(.LANCHOR7)
	addi	a0,a0,%lo(.LANCHOR7)
	call	adc_q_push.part.0
.L93:
	andi	s0,s0,2
	beqz	s0,.L91
	li	a5,1073872896
	li	a4,2
	sw	a4,4(a5)
	lui	a5,%hi(.LANCHOR3)
	lbu	a5,%lo(.LANCHOR3)(a5)
	andi	a5,a5,0xff
	beqz	a5,.L91
	lui	a0,%hi(.LANCHOR7+120)
	addi	a0,a0,%lo(.LANCHOR7+120)
	call	adc_q_push.part.0
.L91:
	lw	s0,12(sp)
	addi	sp,sp,16
	mret
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
	.ident	"GCC: (xPack GNU RISC-V Embedded GCC, 32-bit) 8.2.0"
