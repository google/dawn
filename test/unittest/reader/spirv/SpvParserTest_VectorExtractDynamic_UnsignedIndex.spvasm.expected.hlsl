SKIP: FAILED

void main_1() {
  const uint2 x_1 = uint2(3u, 4u);
  const uint x_10 = x_1[3u];
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
tint_kzPJNg:3:25: error: vector element index '3' is out of bounds
  const uint x_10 = x_1[3u];
                        ^


