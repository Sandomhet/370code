        lw      0       1       pos5    reg1 = 5
        lw      0       2       neg3    reg2 = -3
        add     1       2       3       reg3 = 2, add across signs
        nor     1       1       4       reg4 = ~5 = -6
        nor     0       0       5       reg5 = ~0 = -1
        nor     1       2       6       reg6 = ~(5|-3) = 2
        add     4       5       7       reg7 = -7
        halt
pos5    .fill   5
neg3    .fill   -3
