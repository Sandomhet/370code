        lw      0       1       one     r1 = 1
        noop
        noop
        noop
        add     1       0       2       r2 = 1 (stale 0)
        beq     1       2       tgt1    taken only with forwarding from EX/MEM
        add     1       1       3       squashed
        add     1       1       4       squashed
        add     1       1       5       squashed
tgt1    add     2       2       2       r2 = 2
        noop
        beq     1       2       bad     not taken only with forwarding (stale 1)
        add     2       1       6       r6 = 3
        noop
        noop
        beq     6       2       bad     r6 from WB/END... r2 regfile, not taken
        add     6       0       7       r7 = 3
        add     1       1       3       r3 = 2
        beq     3       2       fwd     taken, A from EX/MEM: skip halt
        halt                            squashed
bad     add     0       0       0       must never run
        halt
fwd     beq     0       0       fwd2    taken, back-to-back branch
        beq     0       0       bad     squashed (would be taken)
        noop
        noop
fwd2    beq     0       0       next    taken with offset 0, refetches next
next    add     7       7       4       r4 = 6
        halt
one     .fill   1
