int get_call_count(void)
{
    static int count = 0;
    count += 1;
    return count;
}

int i = 2;
int j = 1;
int k = 0;

int main(void)
{
    int arr[3][2][2] = {
        {{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}, {{9, 10}, {11, 12}}};

    if (arr[i][j][k]++ != 11)
    {
        return 1;
    }

    if (arr[i][j][k] != 12)
    {
        return 2;
    }

    if (++arr[--i][j--][++k] != 9)
    {
        return 3;
    }

    arr[1][2][1] &= arr[3][2][1];
    arr[1][3][1] |= arr[1][2][1];
    arr[2][1][3] ^= arr[1][2][1];
    arr[2][1][3] >>= 3;

    double x[5] = {0.0, 1.0, 2.0, 3.0, 4.0};
    double *ptr = x;

    if (*ptr != 1.0)
    {
        return 2;
    }

    if (ptr++ != x + 1)
    {
        return 3;
    }

    double *end_ptr = x + 1;
    end_ptr[1] += 3;
}