
RWByteAddressBuffer arr : register(u0);
uint f2() {
  uint v = 0u;
  arr.GetDimensions(v);
  return (v / 4u);
}

uint f1() {
  return f2();
}

uint f0() {
  return f1();
}

[numthreads(1, 1, 1)]
void main() {
  uint v_1 = 0u;
  arr.GetDimensions(v_1);
  uint v_2 = (min(0u, ((v_1 / 4u) - 1u)) * 4u);
  arr.Store((0u + v_2), f0());
}

