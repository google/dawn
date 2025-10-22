
int f(int a, int b, int c) {
  return asint((asuint(asint((asuint(a) * asuint(b)))) + asuint(c)));
}

[numthreads(1, 1, 1)]
void main() {
  f(int(1), int(2), int(3));
}

