static unsigned long global_var;

int switch_on_uint(unsigned int ui) {
    switch (ui) {
        case 5u:
            return 0;
        // this will be converted to an unsigned int, preserving its value
        case 4294967286l:
            return 1;
        // 2^35 + 10, will be converted to 10
        case 34359738378ul:
            return 2;
        default:
            return 3;
    }
}

int main(void) {
    unsigned u = 2147483647u;
    int res = (u + 2u == 2147483649u);

    static unsigned long shiftcount = 5;
    res >>= shiftcount;

    if (switch_on_uint(5) != 0)
        return res * 12;
    if (switch_on_uint(4294967286) != 1)
        return res + 2ul;
    if (switch_on_uint(10) != 2)
        return res - 3ul;

    return global_var;
}