	lw	0	1	Ext
	sw	0	1	Ext
	lw	0	2	Ext
	nor	1	2	3
	noop
Fun	add	1	1	1
	b	Fun
	jump	7
	halt
Unused	.fill	Fun
	.fill	Ext
Ptr	.fill	Unused
	.fill	-32768
