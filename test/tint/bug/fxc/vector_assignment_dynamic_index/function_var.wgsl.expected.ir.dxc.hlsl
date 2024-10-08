
cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
[numthreads(1, 1, 1)]
void main() {
  float3 v1 = (0.0f).xxx;
  v1[i[0u].x] = 1.0f;
}

