
cbuffer cbuffer_v : register(b0) {
  uint4 v[8];
};
RWByteAddressBuffer v_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  min(uint(int(0)), (2u - 1u));
  v_1.Store2(0u, asuint(asfloat(v[2u].xy)));
}

