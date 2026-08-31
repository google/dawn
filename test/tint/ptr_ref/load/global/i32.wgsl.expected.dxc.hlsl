
static int I = int(0);
[numthreads(1, 1, 1)]
void main() {
  int i = I;
  int u = asint((asuint(i) + 1u));
}

