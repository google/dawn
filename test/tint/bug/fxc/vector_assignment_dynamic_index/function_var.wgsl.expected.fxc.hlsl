
cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
[numthreads(1, 1, 1)]
void main() {
  float3 v1 = (0.0f).xxx;
  float3 v = v1;
  v1 = (((uint3((i[0u].x).xxx) == uint3(0u, 1u, 2u))) ? ((1.0f).xxx) : (v));
}

