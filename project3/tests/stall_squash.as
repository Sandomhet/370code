        lw      0       1       one     r1 = 1
        noop
        noop
        noop
        beq     0       0       tgt     taken; reaches MEM while lw/add stall
        lw      0       2       big     squashed
        add     2       2       3       squashed (would stall)
        add     1       1       4       squashed
tgt     lw      0       5       big     r5 = 100
        sw      0       5       slot    stall on data: slot = 100
        lw      0       6       slot    r6 = 100
        beq     5       6       end     stall on beq regB, taken
        halt                            squashed
        add     1       1       7       squashed
end     add     6       1       7       r7 = 101
        halt
one     .fill   1
big     .fill   100
slot    .fill   0
