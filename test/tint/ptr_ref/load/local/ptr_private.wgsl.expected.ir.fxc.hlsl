
static int i = 123;
[numthreads(1, 1, 1)]
void main() {
  int p = i;
  int u = (p + 1);
}

