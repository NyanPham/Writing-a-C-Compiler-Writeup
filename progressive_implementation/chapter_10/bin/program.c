static int foo;

int main(void)
{
    for (int i = 0; i < 10; i++)
    {
        extern int inc;
        foo += inc;

        if (foo > 10)
        {
            break;
        }

        if (foo == 5)
        {
            continue;
        }
    }
    return foo;
}

int inc = 3;
extern int foo;