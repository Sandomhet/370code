        lw      0       1       mcand   reg1 = mcand, doubled on every pass
        lw      0       6       mplier  reg6 = mplier
        lw      0       5       limit   reg5 = 32768, the mask value that ends the loop
        lw      0       4       one     reg4 = 1, mask selecting the current mplier bit
        nor     6       6       6       reg6 = ~mplier, loop invariant used by the AND
        add     0       0       3       reg3 = 0, the running product
loop    nor     4       4       7       reg7 = ~mask
        nor     6       7       7       reg7 = mplier & mask, since ~(~a|~b) == a&b
        beq     7       0       skip    this mplier bit is 0, so add nothing
        add     3       1       3       product += mcand << i
skip    add     1       1       1       mcand <<= 1
        add     4       4       4       mask <<= 1
        beq     4       5       done    all 15 mplier bits have been consumed
        beq     0       0       loop
done    halt
mcand   .fill   6203
mplier  .fill   1429
limit   .fill   32768
one     .fill   1