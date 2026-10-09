zero0   lw      0       1       abcde6  label defined at address 0
        beq     0       0       L       one-character label
L       sw      0       1       a1
        halt
abcde6  .fill   1               6-character label with digits
a1      .fill   abcde6
addr0   .fill   zero0
