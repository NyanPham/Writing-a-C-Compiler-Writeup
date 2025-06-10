signed char static_array[3][4] = {{'a', 'b', 'c', 'd'}, "efgh", "ijk"};

int main(void)
{
    unsigned char auto_array[2][3] = {"lmn", {'o', 'p'}};

    for (int i = 0; i < 3; i = i + 1)
        for (int j = 0; j < 4; j = j + 1)
            if (static_array[i][j] != "abcdefghijk"[i * 4 + j])
                return 1;

    unsigned char uc = 255;

    // uc is promoted to int, then shifted
    if ((uc >> 3) != 31)
    {
        return 2;
    }

    char c = -56;
    // make sure c << 3ul is promoted to int, not unsigned long
    if (((-(c << 3ul)) >> 3) != -5)
    {
        return 4;
    }

    switch (c)
    {
    // if we reduced this to a char it would be -56
    // but we won't, so this case shouldn't be taken
    case 33554632:
        return 1;
    default:
        return 0;
    }

    char c2 = 100;
    c += c2;

    return 0;
}