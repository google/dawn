
cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};
RWByteAddressBuffer result : register(u1, space1);
RWByteAddressBuffer s : register(u0);
int runTest() {
  uint v = (uint((0u + uint(constants[0u].x))) * 4u);
  int v_1 = int(0);
  s.InterlockedOr(int(0u), int(0), v_1);
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  result.Store(0u, uint(runTest()));
}

