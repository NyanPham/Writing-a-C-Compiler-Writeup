int main(void)
{
    int x = 100;
    x <<= 22l;

    long y = 10000;
    y += x;

    if (x != 419430400)
    {
        return 1; 
    }

    return 0;
}