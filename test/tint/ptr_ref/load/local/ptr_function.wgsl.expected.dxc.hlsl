
[numthreads(1, 1, 1)]
void main() {
  int i = int(123);
  int u = asint((asuint(i) + asuint(int(1))));
}

