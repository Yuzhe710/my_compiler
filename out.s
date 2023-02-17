	.text
	.text
	.globl	fred
	.type	fred, @function
fred:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$20, %r8
	movl	%r8d, %eax
	jmp	L1
L1:
	popq	%rbp
	ret
	.comm	result,4,4
