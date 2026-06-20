# Gerado pelo Compilador SimpleC
# Sintaxe: AT&T/GAS  |  Target: x86-64 Linux
# Compilar: gcc -no-pie saida.asm -o programa


	.section	.text

	.globl	main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp		# 7 variavel(is) * 4 bytes
	# int a = 10
	movl	$10, -4(%rbp)
	# int b = 20
	movl	$20, -8(%rbp)
	# int c = 5
	movl	$5, -12(%rbp)
	# int f = 3
	movl	$3, -16(%rbp)
	# int g = 4
	movl	$4, -20(%rbp)
	# int h = 8
	movl	$8, -24(%rbp)
	# int r = (((f + g) - h) + ((a + b) * (f + g)))
	movl	-16(%rbp), %eax		# f
	pushq	%rax			# salva operando esquerdo
	movl	-20(%rbp), %eax		# g
	popq	%rcx			# restaura operando esquerdo
	addl	%ecx, %eax		# eax = left + right
	pushq	%rax			# salva operando esquerdo
	movl	-24(%rbp), %eax		# h
	popq	%rcx			# restaura operando esquerdo
	subl	%eax, %ecx		# ecx = left - right
	movl	%ecx, %eax
	pushq	%rax			# salva operando esquerdo
	movl	-4(%rbp), %eax		# a
	pushq	%rax			# salva operando esquerdo
	movl	-8(%rbp), %eax		# b
	popq	%rcx			# restaura operando esquerdo
	addl	%ecx, %eax		# eax = left + right
	pushq	%rax			# salva operando esquerdo
	movl	-16(%rbp), %eax		# f
	pushq	%rax			# salva operando esquerdo
	movl	-20(%rbp), %eax		# g
	popq	%rcx			# restaura operando esquerdo
	addl	%ecx, %eax		# eax = left + right
	popq	%rcx			# restaura operando esquerdo
	imull	%ecx, %eax		# eax = left * right
	popq	%rcx			# restaura operando esquerdo
	addl	%ecx, %eax		# eax = left + right
	movl	%eax, -28(%rbp)
	# return r
	movl	-28(%rbp), %eax		# r
	jmp	.Lmain_ret
.Lmain_ret:
	movq	%rbp, %rsp
	popq	%rbp
	ret
