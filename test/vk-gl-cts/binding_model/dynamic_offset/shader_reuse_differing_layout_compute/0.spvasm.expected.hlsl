RWByteAddressBuffer x_4 : register(u1, space0);

void main_1() {
  x_4.Store4(0u, asuint(float4(1.0f, 2.0f, 3.0f, 4.0f)));
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
