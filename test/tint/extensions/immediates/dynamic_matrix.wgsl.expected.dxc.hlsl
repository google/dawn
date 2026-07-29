
cbuffer cbuffer_pc : register(b0, space1) {
  uint4 pc[1];
};
RWByteAddressBuffer v : register(u0);
cbuffer cbuffer_idx : register(b1) {
  uint4 idx[1];
};
[numthreads(1, 1, 1)]
void main() {
  uint v_1 = (min(idx[0u].x, 1u) * 8u);
  uint4 v_2 = pc[(v_1 / 16u)];
  v.Store2(0u, asuint(asfloat(select((((v_1 & 15u) >> 2u) == 2u), v_2.zw, v_2.xy))));
}

