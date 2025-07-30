// Each variable below should be assigned a register

int add(int a, int b) {
    int c = a + b;  // c: sum of inputs
    int d = c * 2;  // d: double the sum
    int e = d - a;  // e: difference between double-sum and first input
    int f = e + b;  // f: add second input to previous result
    int g = f * c;  // g: multiply previous result by sum

    // Register coalescing test: chain of assignments
    int h = g;      // h should ideally share register with g
    int i = h;      // i should ideally share register with h/g
    int j = i + 1;  // j depends on i, coalescing possible

    // More coalescing: copy propagation
    int k = j;
    int l = k;
    int m = l * 2;

    // Return a value that depends on the coalesced chain
    return m;
}

int main(void) {
    int x = 5;
    int y = 7;
    int result = add(x, y);
    return 0;
}