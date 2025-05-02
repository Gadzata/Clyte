struct Pair
{
    int x, y;
};

struct Triple
{
    int x, y, z;
};

struct Quad
{
    int a, b, c, d;
};

int compute_pair(struct Pair p)
{
    return p.x + p.y;
}

int compute_triple(struct Triple t)
{
    return t.x + t.y + t.z;
}

int compute_quad(struct Quad q)
{
    return q.a * q.b + q.c - q.d;
}

int factorial(int n)
{
    int result = 1;
    int i = 1;
    for (i = 1; i <= n; i = i + 1)
    {
        result = result * i;
    }
    return result;
}

int heavy_calculation(int base)
{
    int result = 0;
    int i = 0;
    for (i = 1; i <= 50; i = i + 1)
    {
        result = result + factorial(i / 10 + 1);
    }
    return result + base;
}

int main()
{
    int total = 0;
    int i = 0;

    for (i = 0; i < 500; i = i + 1)
    {
        struct Pair p;
        struct Triple t;
        struct Quad q;

        p.x = i;
        p.y = i * 2;

        t.x = i;
        t.y = i + 1;
        t.z = i + 2;

        q.a = i;
        q.b = i + 1;
        q.c = i + 2;
        q.d = i + 3;

        total = total + compute_pair(p);
        total = total + compute_triple(t);
        total = total + compute_quad(q);
        total = total + heavy_calculation(i);
    }

    return total;
}