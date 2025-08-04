
cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
[numthreads(1, 1, 1)]
void main() {
  float3 v1 = (0.0f).xxx;
  uint v = i[0u].x;
  float3 v_1 = v1;
  float3 v_2 = float3((1.0f).xxx);
  uint3 v_3 = uint3((v).xxx);
  v1 = (((v_3 == uint3(0u, 1u, 2u))) ? (v_2) : (v_1));
}

