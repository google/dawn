cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};

RWByteAddressBuffer result : register(u1, space1);

RWByteAddressBuffer s : register(u0, space0);

int tint_atomicLoad(RWByteAddressBuffer buffer, uint offset) {
  int value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}


int runTest() {
  return tint_atomicLoad(s, (4u * (0u + uint(constants[0].x))));
}

[numthreads(1, 1, 1)]
void main() {
  const int tint_symbol = runTest();
  const uint tint_symbol_1 = uint(tint_symbol);
  result.Store(0u, asuint(tint_symbol_1));
  return;
}
