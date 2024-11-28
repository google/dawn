
TextureCube<float4> t_f : register(t0);
TextureCube<int4> t_i : register(t1);
TextureCube<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  t_f.GetDimensions(0u, v.x, v.y, v.z);
  uint3 v_1 = (0u).xxx;
  t_f.GetDimensions(uint(min(uint(int(1)), (v.z - 1u))), v_1.x, v_1.y, v_1.z);
  uint2 fdims = v_1.xy;
  uint3 v_2 = (0u).xxx;
  t_i.GetDimensions(0u, v_2.x, v_2.y, v_2.z);
  uint3 v_3 = (0u).xxx;
  t_i.GetDimensions(uint(min(uint(int(1)), (v_2.z - 1u))), v_3.x, v_3.y, v_3.z);
  uint2 idims = v_3.xy;
  uint3 v_4 = (0u).xxx;
  t_u.GetDimensions(0u, v_4.x, v_4.y, v_4.z);
  uint3 v_5 = (0u).xxx;
  t_u.GetDimensions(uint(min(uint(int(1)), (v_4.z - 1u))), v_5.x, v_5.y, v_5.z);
  uint2 udims = v_5.xy;
}

