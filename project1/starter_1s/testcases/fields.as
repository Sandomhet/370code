        lw      0       1       n1      reg1 = 1
        noop                            noop must only advance the pc
        lw      0       2       n2      reg2 = 2
        lw      0       3       n4      reg3 = 4
        add     1       2       4       reg4 = 3, destReg differs from both sources
        noop
        add     2       3       5       reg5 = 6
        nor     1       3       6       reg6 = ~(1|4) = -6
        add     4       5       7       reg7 = 9
        halt
n1      .fill   1
n2      .fill   2
n4      .fill   4
