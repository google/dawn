[numthreads(1, 1, 1)]
void f() {
  uint a = 4u;
  uint3 b = uint3(0u, 2u, 0u);
  uint3 r = (a / (b + b));
}

