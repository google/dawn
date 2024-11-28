
TextureCubeArray t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint4 v = (0u).xxxx;
  t_f.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  t_f.GetDimensions(uint(min(uint(int(0)), (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  uint2 dims = v_1.xy;
}

