        lw      0       1       a       prefix-colliding label names
        lw      0       2       ab
        lw      0       3       abc
self    beq     0       0       self    branches to itself, offset -1
        halt
a       .fill   1
ab      .fill   2
abc     .fill   3
