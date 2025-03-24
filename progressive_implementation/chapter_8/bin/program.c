int main(void) {
    int a = -2147483647;

    while (a < 5)
    {
        if (a == -10) {
            a = a + 1;
            continue;
        }
        
        a = a + 2;
    }

    do {
        a = a * 2;
        if (a == 6)  break;
    } while (a < 11);

    for (a = 3; a % 5 != 0; a = a + 1)
        a = a + 1;

    return a % 5 || a > 0;
}
