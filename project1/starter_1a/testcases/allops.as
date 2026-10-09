        add     1       2       3       every opcode, distinct register fields
        nor     4       5       7       destReg = 7
        add     7       0       0       destReg = 0
        lw      7       1       tgt
        sw      2       3       tgt
        beq     4       5       tgt
        jalr    6       7
        noop
        halt
tgt     .fill   7
