        lw      0       1       one     reg1 = 1
        lw      0       2       two     reg2 = 2
        lw      0       3       neg1    reg3 = -1
        beq     1       2       bad     not taken, 1 != 2
        beq     1       1       fwd     taken forward, target is pc+1+offset
        add     1       1       4       skipped, reg4 stays 0
bad     add     2       2       5       skipped, reg5 stays 0
        halt                            skipped
fwd     add     1       2       6       reg6 = 3, loop counter
        add     0       0       7       reg7 = 0, accumulator
loop    beq     6       0       done    exit once the counter hits 0
        add     6       3       6       counter -= 1
        add     7       1       7       accumulator += 1
        beq     0       0       loop    always taken, negative offset
done    halt
one     .fill   1
two     .fill   2
neg1    .fill   -1
