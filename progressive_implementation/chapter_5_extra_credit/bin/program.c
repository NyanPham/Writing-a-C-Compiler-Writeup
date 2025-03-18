int main(void) {
    int a = 5;
    int b = 2;
    int c = 1;
  
    b += a++;
    b--;
    c *= a;
    b += c;
    a >>= b;
    c %= a;
    a += b-- * c++;
  
    return a + b + c;
  }