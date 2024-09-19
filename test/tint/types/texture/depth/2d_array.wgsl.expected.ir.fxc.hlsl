
Texture2DArray t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint4 v = (0u).xxxx;
  t_f.GetDimensions(uint(int(0)), v[0u], v[1u], v[2u], v[3u]);
  uint2 dims = v.xy;
}

