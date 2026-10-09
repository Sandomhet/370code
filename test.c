#include <stdio.h>

int numHighBits(int num) {
    int n = sizeof(num);
    int cnt = 0;
    for (int i = 0; i < n; i++) {
        cnt += (num & 1);
        num >>= 1;
    }
    return cnt;
}

int main() {
    int x;
    scanf("%d", &x);
    printf("%d\n", numHighBits(x));
}