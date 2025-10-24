
static int i = int(0);
[numthreads(1, 1, 1)]
void main() {
  i = asint((asuint(i) + asuint(int(1))));
}

