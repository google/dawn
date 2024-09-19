
TextureCube<float4> t_f : register(t0);
TextureCube<int4> t_i : register(t1);
TextureCube<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  t_f.GetDimensions(uint(int(1)), v[0u], v[1u], v[2u]);
  uint2 fdims = v.xy;
  uint3 v_1 = (0u).xxx;
  t_i.GetDimensions(uint(int(1)), v_1[0u], v_1[1u], v_1[2u]);
  uint2 idims = v_1.xy;
  uint3 v_2 = (0u).xxx;
  t_u.GetDimensions(uint(int(1)), v_2[0u], v_2[1u], v_2[2u]);
  uint2 udims = v_2.xy;
}

