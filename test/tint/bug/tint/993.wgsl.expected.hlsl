int atomicLoad_1(RWByteAddressBuffer buffer, uint offset) {
  int value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}

cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};

RWByteAddressBuffer result : register(u1, space1);

RWByteAddressBuffer s : register(u0, space0);

int runTest() {
  return atomicLoad_1(s, (4u * (0u + uint(constants[0].x))));
}

[numthreads(1, 1, 1)]
void main() {
  result.Store(0u, asuint(uint(runTest())));
  return;
}
