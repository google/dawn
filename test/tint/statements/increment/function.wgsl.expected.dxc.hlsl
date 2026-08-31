
[numthreads(1, 1, 1)]
void main() {
  int i = int(0);
  i = asint((asuint(i) + 1u));
}

