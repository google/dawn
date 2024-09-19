
TextureCube t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  t_f.GetDimensions(uint(int(0)), v[0u], v[1u], v[2u]);
  uint2 dims = v.xy;
}

