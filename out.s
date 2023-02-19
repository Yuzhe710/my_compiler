	.text
	.comm	a,1,1
	.comm	b,0,0
	.comm	c,1,1
	.comm	d,4,4
	.comm	e,0,0
	.comm	f,4,4
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$18, %r8
	movb	%r8b, a(%rip)
	movzbq	a(%rip), %r8
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	leaq	a(%rip), %r8
	movq	%r8, b(%rip)
	movq	b(%rip), %r8
	movzbq	(%r8), %r8
	movb	%r8b, c(%rip)
	movzbq	c(%rip), %r8
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
