
cbuffer cbuffer_pc : register(b0, space1) {
  uint4 pc[1];
};
RWByteAddressBuffer v : register(u0);
cbuffer cbuffer_idx : register(b1) {
  uint4 idx[1];
};
[numthreads(1, 1, 1)]
void main() {
  uint v_1 = (min(idx[0u].x, 1u) * 4u);
  v.Store(0u, asuint(asfloat(pc[(v_1 / 16u)][((v_1 & 15u) >> 2u)])));
}

