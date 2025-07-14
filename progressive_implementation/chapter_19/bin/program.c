void printf(char *str);

int compute(int x) {
    int a = 2 + 3;            // constant folding: 2+3 → 5
    int b = a + x;            // copy propagation candidate
    int c = b * 1;            // dead store: c is never used

    if (0) {                  // unreachable code
        printf("This will never be printed.\n");
    }

    int d = b;                // copy propagation: d = b
    int e = d + 0;            // constant folding: d + 0 → d
    int f = e;                // dead store: f is never used

    int result = d * 2;       // used value
    return result;
}

int main(void) {
    int x = 10;
    int output = compute(x);
    return output;
}