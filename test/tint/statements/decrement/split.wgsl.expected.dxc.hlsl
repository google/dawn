
[numthreads(1, 1, 1)]
void main() {
  int b = int(2);
  int c = asint((asuint(b) - asuint(asint((~(asuint(b)) + 1u)))));
}

