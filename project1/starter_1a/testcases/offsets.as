        lw      0       1       32767   max positive offsetField
        lw      0       2       -32768  min negative offsetField
        sw      3       4       -1      negative numeric, must mask to 16 bits
        beq     0       0       0       zero offset
back    beq     1       1       -3      negative numeric branch
        beq     0       0       back    negative symbolic branch
        beq     0       0       fwd     forward symbolic branch
fwd     halt
