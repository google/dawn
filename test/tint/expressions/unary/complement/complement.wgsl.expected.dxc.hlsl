
int i(int x) {
  return ~(x);
}

uint u(uint x) {
  return ~(x);
}

int4 vi(int4 x) {
  return ~(x);
}

uint4 vu(uint4 x) {
  return ~(x);
}

[numthreads(1, 1, 1)]
void main() {
  i(int(1));
  u(1u);
  vi((int(0)).xxxx);
  vu((0u).xxxx);
}

