
Texture2D t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  t_f.GetDimensions(0u, v.x, v.y, v.z);
  uint3 v_1 = (0u).xxx;
  t_f.GetDimensions(uint(min(uint(int(0)), (v.z - 1u))), v_1.x, v_1.y, v_1.z);
  uint2 dims = v_1.xy;
}

