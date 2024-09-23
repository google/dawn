
Texture3D<float4> t_f : register(t0);
Texture3D<int4> t_i : register(t1);
Texture3D<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint4 v = (0u).xxxx;
  t_f.GetDimensions(uint(int(1)), v[0u], v[1u], v[2u], v[3u]);
  uint3 fdims = v.xyz;
  uint4 v_1 = (0u).xxxx;
  t_i.GetDimensions(uint(int(1)), v_1[0u], v_1[1u], v_1[2u], v_1[3u]);
  uint3 idims = v_1.xyz;
  uint4 v_2 = (0u).xxxx;
  t_u.GetDimensions(uint(int(1)), v_2[0u], v_2[1u], v_2[2u], v_2[3u]);
  uint3 udims = v_2.xyz;
}

