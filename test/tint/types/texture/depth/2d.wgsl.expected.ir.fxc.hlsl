
Texture2D t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  Texture2D v = t_f;
  uint3 v_1 = (0u).xxx;
  v.GetDimensions(uint(int(0)), v_1[0u], v_1[1u], v_1[2u]);
  uint2 dims = v_1.xy;
}

