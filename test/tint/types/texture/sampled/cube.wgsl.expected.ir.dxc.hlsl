
TextureCube<float4> t_f : register(t0);
TextureCube<int4> t_i : register(t1);
TextureCube<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  TextureCube<float4> v = t_f;
  uint3 v_1 = (0u).xxx;
  v.GetDimensions(uint(1), v_1[0u], v_1[1u], v_1[2u]);
  uint2 fdims = v_1.xy;
  TextureCube<int4> v_2 = t_i;
  uint3 v_3 = (0u).xxx;
  v_2.GetDimensions(uint(1), v_3[0u], v_3[1u], v_3[2u]);
  uint2 idims = v_3.xy;
  TextureCube<uint4> v_4 = t_u;
  uint3 v_5 = (0u).xxx;
  v_4.GetDimensions(uint(1), v_5[0u], v_5[1u], v_5[2u]);
  uint2 udims = v_5.xy;
}

