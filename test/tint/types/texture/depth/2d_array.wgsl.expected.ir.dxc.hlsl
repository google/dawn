
Texture2DArray t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  Texture2DArray v = t_f;
  uint4 v_1 = (0u).xxxx;
  v.GetDimensions(uint(0), v_1[0u], v_1[1u], v_1[2u], v_1[3u]);
  uint2 dims = v_1.xy;
}

