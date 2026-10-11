Start	lw	0	1	Here
	lw	0	2	loc
	sw	0	1	Stack
	lw	0	3	Stack
	beq	0	0	end
end	halt
Here	.fill	Start
loc	.fill	end
