        jump    3               jalr 3 0
        b       skip            beq 0 0 skip
        noop    1       2       3       unused fields must be zeroed
skip    halt    4       5       6
        jalr    6       7       5
        jalr    5       5               regA == regB
