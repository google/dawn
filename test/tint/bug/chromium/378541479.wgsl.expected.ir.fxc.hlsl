
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
  uint3 v_1 = (0u).xxx;
  tex.GetDimensions(0u, v_1.x, v_1.y, v_1.z);
  uint v_2 = min(level[0u].x, (v_1.z - 1u));
  uint3 v_3 = (0u).xxx;
  tex.GetDimensions(uint(v_2), v_3.x, v_3.y, v_3.z);
  int2 v_4 = int2(min(v, (v_3.xy - (1u).xx)));
  float res = tex.Load(int3(v_4, int(v_2))).x;
}

