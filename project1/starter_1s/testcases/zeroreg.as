        lw      0       1       big     reg1 = 99
        add     1       1       0       add cannot write reg0
        add     0       1       2       reg2 = 99, proves reg0 still reads 0
        nor     1       1       0       nor cannot write reg0
        add     0       0       3       reg3 = 0
        lw      0       0       big     lw cannot write reg0
        add     0       1       4       reg4 = 99
        sw      0       0       slot    mem[slot] = 0, overwrites 77
        lw      0       5       slot    reg5 = 0
        beq     0       3       tgt     taken, both operands are 0
        add     1       1       6       skipped, reg6 stays 0
tgt     halt
big     .fill   99
slot    .fill   77
