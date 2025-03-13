#ifdef SUPPRESS_WARNINGS
#pragma GCC diagnostic ignored "-Wparentheses"
#endif

int main(void) {
    return 20 & 7 >> 4 <= 3 ^ 7 < 5 | 7 != 5;
}