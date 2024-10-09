
cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
static float3 v1 = (0.0f).xxx;
[numthreads(1, 1, 1)]
void main() {
  float3 v = v1;
  float3 v_1 = i[0u].x.xxx;
  v1 = (((v_1 == float3(int(0), int(1), int(2)))) ? (1.0f.xxx) : (v));
}

