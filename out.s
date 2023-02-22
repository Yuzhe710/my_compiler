	.text
	.comm	d,4,4
	.comm	f,4,4
	.comm	e,8,8
	.comm	a,4,4
	.comm	b,4,4
	.comm	c,4,4
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$3, %r8
	movl	%r8d, b(%rip)
	movq	$5, %r8
	movl	%r8d, c(%rip)
	movzbl	b(%rip), %r8d
	movzbl	c(%rip), %r9d
	movq	$10, %r10
	imulq	%r9, %r10
	addq	%r8, %r10
	movl	%r10d, a(%rip)
	movzbl	a(%rip), %r8d
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	movq	$12, %r8
	movl	%r8d, d(%rip)
	movzbl	d(%rip), %r8d
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	leaq	d(%rip), %r8
	movq	%r8, e(%rip)
	movq	e(%rip), %r8
	movq	(%r8), %r8
	movl	%r8d, f(%rip)
	movzbl	f(%rip), %r8d
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	movq	$0, %r8
	movl	%r8d, %eax
	jmp	L1
L1:
	popq	%rbp
	ret
