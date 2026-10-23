	lw	0	1	n
	lw	0	2	r
	lw	0	4	Faddr	load function address
	jalr	4	7		call function
	halt
comb	beq	2	0	base	if r == 0, return 1
	beq	1	2	base	if n == r, return 1
	lw	0	6	pos1	r6 = 1
	sw	5	7	Stack	push return address
	add	5	6	5
	sw	5	1	Stack	push n
	add	5	6	5
	sw	5	2	Stack	push r
	add	5	6	5
	sw	5	4	Stack	push r4 (caller's local)
	add	5	6	5
	lw	0	6	neg1	r6 = -1
	add	1	6	1	n = n - 1
	lw	0	6	Faddr
	jalr	6	7		r3 = combination(n-1, r)
	add	3	0	4	r4 = combination(n-1, r)
	lw	0	6	neg1	r6 = -1
	add	2	6	2	r = r - 1 (callee left r1 = n-1)
	lw	0	6	Faddr
	jalr	6	7		r3 = combination(n-1, r-1)
	add	4	3	3	r3 = combination(n-1, r) + combination(n-1, r-1)
	lw	0	6	neg1	r6 = -1
	add	5	6	5
	lw	5	4	Stack	pop r4
	add	5	6	5
	lw	5	2	Stack	pop r
	add	5	6	5
	lw	5	1	Stack	pop n
	add	5	6	5
	lw	5	7	Stack	pop return address
	jalr	7	6		return
base	lw	0	3	pos1	r3 = 1
	jalr	7	6		return
n	.fill	7
r	.fill	3
Faddr	.fill	comb
pos1	.fill	1
neg1	.fill	-1
