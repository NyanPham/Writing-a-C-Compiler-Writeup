// Test that we can use coalescing to prevent spills. In particular,

int glob = 5;
int flag = 0;

int validate(int a, int b, int c, int d, int e, int f, int g, int h, int i,
			 int j, int k, int l, int m);

int target(int arg)
{
	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
	int g;
	int h;
	int i;
	int j;
	int k;
	int l;
	int m;

	if (flag)
	{
		a = arg;
		b = arg;
		c = arg;
		d = arg;
		e = arg;
		f = arg;
		g = arg;
		h = arg;
		i = arg;
		j = arg;
		k = arg;
		l = arg;
		m = arg;
	}
	else
	{
		a = glob * 2; // 10
		b = a;
		c = a;
		d = a;
		e = a;
		f = a;
		g = a;
		h = a;
		i = a;
		j = a;
		k = a;
		l = a;
		m = a;
	}
	return validate(a, b, c, d, e, f, g, h, i, j, k, l, m);
}