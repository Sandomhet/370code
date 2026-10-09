Start	lw	0	1	-32768
	sw	0	1	32767
	lw	1	2	Start
	beq	1	2	end
	beq	0	0	Start
end	halt
