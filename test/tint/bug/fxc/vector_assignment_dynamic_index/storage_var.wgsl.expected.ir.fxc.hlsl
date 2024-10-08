
cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
RWByteAddressBuffer v1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  uint v = (uint(i[0u].x) * 4u);
  v1.Store((0u + v), asuint(1.0f));
}

