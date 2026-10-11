Main	lw	0	1	Val
	lw	0	4	FnAdr
	jalr	4	7
	sw	0	3	Res
	lw	0	2	ptr
	lw	2	5	0
	lw	0	6	Ptr
	lw	0	7	Sp
	halt
Res	.fill	0
ptr	.fill	Res
