
TextureCubeArray<float4> t_f : register(t0);
TextureCubeArray<int4> t_i : register(t1);
TextureCubeArray<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint4 v = (0u).xxxx;
  t_f.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  t_f.GetDimensions(uint(min(uint(int(1)), (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  uint2 fdims = v_1.xy;
  uint4 v_2 = (0u).xxxx;
  t_i.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint4 v_3 = (0u).xxxx;
  t_i.GetDimensions(uint(min(uint(int(1)), (v_2.w - 1u))), v_3.x, v_3.y, v_3.z, v_3.w);
  uint2 idims = v_3.xy;
  uint4 v_4 = (0u).xxxx;
  t_u.GetDimensions(0u, v_4.x, v_4.y, v_4.z, v_4.w);
  uint4 v_5 = (0u).xxxx;
  t_u.GetDimensions(uint(min(uint(int(1)), (v_4.w - 1u))), v_5.x, v_5.y, v_5.z, v_5.w);
  uint2 udims = v_5.xy;
}

