        lw      0       1       fadr    reg1 = address of func
        lw      0       2       five    reg2 = 5
        jalr    1       7               reg7 = 3 (pc+1), then jump to func
        add     2       2       3       reg3 = 10, this is the return point
        jalr    3       3               regA == regB, so this jumps to pc+1
        lw      0       4       eadr    reg4 = address of endf
        jalr    4       0               reg0 stays 0, still jumps to endf
        halt                            skipped
func    add     2       2       6       reg6 = 10
        jalr    7       5               reg5 = 10, return to reg7
endf    halt
fadr    .fill   func
five    .fill   5
eadr    .fill   endf
