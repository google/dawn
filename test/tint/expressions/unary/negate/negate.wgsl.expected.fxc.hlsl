
int i(int x) {
  return asint((~(asuint(x)) + 1u));
}

int4 vi(int4 x) {
  return asint((~(asuint(x)) + (1u).xxxx));
}

[numthreads(1, 1, 1)]
void main() {
  i(int(1));
  vi((int(0)).xxxx);
}

