
cbuffer cbuffer_level : register(b0) {
  uint4 level[1];
};
cbuffer cbuffer_coords : register(b1) {
  uint4 coords[1];
};
Texture2D tex : register(t2);
[numthreads(1, 1, 1)]
void compute_main() {
  uint2 v = coords[0u].xy;
  uint v_1 = level[0u].x;
  uint3 v_2 = (0u).xxx;
  tex.GetDimensions(0u, v_2[0u], v_2[1u], v_2[2u]);
  uint v_3 = min(v_1, (v_2.z - 1u));
  uint3 v_4 = (0u).xxx;
  tex.GetDimensions(uint(v_3), v_4[0u], v_4[1u], v_4[2u]);
  int2 v_5 = int2(min(v, (v_4.xy - (1u).xx)));
  float res = tex.Load(int3(v_5, int(v_3))).x;
}

