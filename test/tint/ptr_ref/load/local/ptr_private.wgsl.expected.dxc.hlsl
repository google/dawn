
static int i = int(123);
[numthreads(1, 1, 1)]
void main() {
  int u = asint((asuint(i) + 1u));
}

