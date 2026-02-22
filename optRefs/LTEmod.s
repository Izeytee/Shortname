	.file	"LTEmod.c"
	.option pic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_v1p0_zicsr2p0_zifencei2p0_zbkb1p0_zve32f1p0_zve32x1p0_zve64d1p0_zve64f1p0_zve64x1p0_zvl128b1p0_zvl32b1p0_zvl64b1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	modulation
	.type	modulation, @function
modulation:
.LFB0:
	.cfi_startproc
	beq	a2,zero,.L8
	slli	a5,a2,32
	srli	a2,a5,29
	add	a3,a1,a2
	lla	a2,.LANCHOR0
.L3:
	lbu	a5,0(a0)
	addi	a1,a1,8
	addi	a0,a0,1
	slli	a5,a5,3
	add	a5,a2,a5
	lw	a4,0(a5)
	sw	a4,-8(a1)
	lw	a5,4(a5)
	sw	a5,-4(a1)
	bne	a1,a3,.L3
.L8:
	ret
	.cfi_endproc
.LFE0:
	.size	modulation, .-modulation
	.align	1
	.globl	modulation_opt
	.type	modulation_opt, @function
modulation_opt:
.LFB2:
	.cfi_startproc
	beq	a2,zero,.L17
	flw	fa4,.LC0,a5
	lla	a6,.LANCHOR0
	slli	a5,a2,32
	srli	a2,a5,29
	add	a2,a1,a2
.L12:
	lbu	a5,0(a0)
	addi	a1,a1,8
	fsw	fa4,-4(a1)
	sraiw	a4,a5,1
	andi	a4,a4,5
	andi	a5,a5,5
	srliw	a3,a4,1
	or	a4,a4,a3
	srliw	a3,a5,1
	andi	a4,a4,3
	or	a5,a5,a3
	slliw	a4,a4,2
	andi	a5,a5,3
	or	a5,a5,a4
	slli	a5,a5,2
	add	a5,a6,a5
	flw	fa5,128(a5)
	fsw	fa5,-8(a1)
	bne	a2,a1,.L12
.L17:
	ret
	.cfi_endproc
.LFE2:
	.size	modulation_opt, .-modulation_opt
	.align	1
	.globl	modulation_full_opt
	.type	modulation_full_opt, @function
modulation_full_opt:
.LFB3:
	.cfi_startproc
	addi	sp,sp,-48
	.cfi_def_cfa_offset 48
	la	a7,__stack_chk_guard
	ld	a5, 0(a7)
	sd	a5, 24(sp)
	li	a5, 0
	sd	ra,40(sp)
	.cfi_offset 1, -8
	vsetivli	zero,4,e8,mf4,ta,ma
	lla	a4,.LANCHOR0+144
	lla	a5,.LANCHOR0+128
	vle32.v	v5,0(a4)
	vle32.v	v6,0(a5)
	beq	a3,zero,.L18
	li	a5,0
	addi	a4,sp,8
	li	a6,18
	j	.L20
.L28:
	vsetvli	zero,zero,e8,mf4,ta,ma
.L20:
	vle8.v	v3,0(a0)
	addiw	a5,a5,4
	vsrl.vi	v4,v3,1
	vand.vi	v3,v3,5
	vand.vi	v4,v4,5
	vsrl.vi	v2,v3,1
	vsrl.vi	v1,v4,1
	vor.vv	v2,v2,v3
	vor.vv	v1,v1,v4
	vand.vi	v2,v2,3
	vand.vi	v1,v1,3
	vsll.vi	v1,v1,2
	vor.vv	v1,v1,v2
	vsetvli	zero,zero,e32,m1,ta,ma
	vzext.vf4	v2,v1
	vse32.v	v2,0(a4)
	vle32.v	v1,0(a4)
	vand.vx	v2,v1,a6
	vand.vi	v1,v1,3
	vsrl.vi	v2,v2,2
	vrgather.vv	v3,v5,v1
	vrgather.vv	v1,v6,v2
	vse32.v	v1,0(a1)
	vse32.v	v3,0(a2)
	bgtu	a3,a5,.L28
.L18:
	ld	a4, 24(sp)
	ld	a5, 0(a7)
	xor	a5, a4, a5
	li	a4, 0
	bne	a5,zero,.L29
	ld	ra,40(sp)
	.cfi_remember_state
	.cfi_restore 1
	addi	sp,sp,48
	.cfi_def_cfa_offset 0
	jr	ra
.L29:
	.cfi_restore_state
	call	__stack_chk_fail@plt
	.cfi_endproc
.LFE3:
	.size	modulation_full_opt, .-modulation_full_opt
	.section	.rodata.cst4,"aM",@progbits,4
	.align	2
.LC0:
	.word	-1082991384
	.section	.rodata
	.align	3
	.set	.LANCHOR0,. + 0
	.type	mod16QAM, @object
	.size	mod16QAM, 128
mod16QAM:
	.word	1050798235
	.word	1050798235
	.word	1050798235
	.word	1064492264
	.word	1064492264
	.word	1050798235
	.word	1064492264
	.word	1064492264
	.word	1050798235
	.word	-1096685413
	.word	1050798235
	.word	-1082991384
	.word	1064492264
	.word	-1096685413
	.word	1064492264
	.word	-1082991384
	.word	-1096685413
	.word	1050798235
	.word	-1096685413
	.word	1064492264
	.word	-1082991384
	.word	1050798235
	.word	-1082991384
	.word	1064492264
	.word	-1096685413
	.word	-1096685413
	.word	-1096685413
	.word	-1082991384
	.word	-1082991384
	.word	-1096685413
	.word	-1082991384
	.word	-1082991384
	.type	mod16QAM_REAL, @object
	.size	mod16QAM_REAL, 16
mod16QAM_REAL:
	.word	-1082991384
	.word	1050798235
	.word	1050798235
	.word	1064492264
	.type	mod16QAM_IMG, @object
	.size	mod16QAM_IMG, 16
mod16QAM_IMG:
	.word	-1082991384
	.word	1050798235
	.word	1050798235
	.word	1064492264
	.ident	"GCC: (Ubuntu 14.2.0-4ubuntu2~24.04) 14.2.0"
	.section	.note.GNU-stack,"",@progbits
