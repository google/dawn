
cbuffer cbuffer_S : register(b0) {
  uint4 S[1];
};
float2 func(uint pointer_indices[1]) {
  uint v = (8u * uint(pointer_indices[0u]));
  uint4 v_1 = S[(v / 16u)];
  return asfloat((((((v % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
}

[numthreads(1, 1, 1)]
void main() {
  uint v_2[1] = {uint(int(1))};
  float2 r = func(v_2);
}

