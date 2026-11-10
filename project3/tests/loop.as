        lw      0       1       count   r1 = 5
        lw      0       2       neg1    r2 = -1
        lw      0       3       base    r3 = base address
loop    beq     1       0       done    counter uses forwarded value
        lw      3       4       0       r4 = mem[r3]
        add     4       5       5       stall: r5 += r4
        add     3       2       3       r3 -= 1 (walk array down)
        add     1       2       1       r1 -= 1
        beq     0       0       loop    always taken, negative offset
        add     6       6       6       squashed every iteration
done    sw      0       5       result  store forwarded sum
        lw      0       6       result  read back
        add     6       6       7       stall: r7 = 2 * sum
        halt
count   .fill   5
neg1    .fill   -1
base    .fill   a5
a1      .fill   1
a2      .fill   2
a3      .fill   3
a4      .fill   4
a5      .fill   5
result  .fill   0
