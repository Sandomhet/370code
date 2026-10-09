Main	lw	0	1	Data1
	sw	0	1	Stack
	lw	0	2	X
	lw	0	3	loc
	lw	0	4	5
	beq	0	1	Main
	jalr	4	7
	halt
Data1	.fill	7
loc	.fill	Main
	.fill	Stack
	.fill	X
	.fill	-3
