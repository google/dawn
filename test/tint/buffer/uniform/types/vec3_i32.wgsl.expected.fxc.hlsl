
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
[numthreads(1, 1, 1)]
void main() {
  int3 x = asint(u[0u].xyz);
  s.Store3(0u, asuint(x));
}

