	.text
	.data
	.globl	c
c:	.byte	0
	.data
	.globl	str
str:	.quad	0
L2:
	.byte	72
	.byte	101
	.byte	108
	.byte	108
	.byte	111
	.byte	32
	.byte	119
	.byte	111
	.byte	114
	.byte	108
	.byte	100
	.byte	10
	.byte	0
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$10, %r8
	movb	%r8b, c(%rip)
	movzbq	c(%rip), %r8
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	leaq	L2(%rip), %r8
	movq	%r8, str(%rip)
L3:
	movq	str(%rip), %r8
	movzbq	(%r8), %r8
	movq	$0, %r9
	cmpq	%r9, %r8
	je	L4
	movq	str(%rip), %r8
	movzbq	(%r8), %r8
	movq	%r8, %rdi
	call	printchar
	movq	%rax, %r9
	movq	str(%rip), %r8
	movq	$1, %r9
	addq	%r8, %r9
	movq	%r9, str(%rip)
	jmp	L3
L4:
	movq	$0, %r8
	movl	%r8d, %eax
	jmp	L1
L1:
	popq	%rbp
	ret
