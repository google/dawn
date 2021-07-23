RWByteAddressBuffer x_4 : register(u3, space0);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
cbuffer cbuffer_x_10 : register(b2, space0) {
  uint4 x_10[1];
};

void main_1() {
  const int x_25 = asint(x_6[0].x);
  x_4.Store(0u, asuint(x_25));
  const int x_28 = asint(x_8[0].x);
  x_4.Store(4u, asuint(x_28));
  const int x_31 = asint(x_10[0].x);
  x_4.Store(8u, asuint(x_31));
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
