
int f(int a, int b, int c) {
  return asint((asuint(asint((asuint(a) * asuint(b)))) + asuint(c)));
}

[numthreads(1, 1, 1)]
void main() {
  int v = f(int(1), int(2), int(3));
  int v_1 = f(int(4), int(5), int(6));
  asint((asuint(v) + asuint(asint((asuint(v_1) * asuint(f(int(7), f(int(8), int(9), int(10)), int(11))))))));
}

