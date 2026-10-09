        lw      0       1       big
        halt
big     .fill   2147483647      upper bound of .fill
small   .fill   -2147483648     lower bound of .fill
zero    .fill   0
selfad  .fill   selfad          label resolving to its own address
alias   .fill   big             backward symbolic fill
fwdref  .fill   tail            forward symbolic fill
tail    .fill   8
