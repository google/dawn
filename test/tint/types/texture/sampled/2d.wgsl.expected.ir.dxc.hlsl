
Texture2D<float4> t_f : register(t0);
Texture2D<int4> t_i : register(t1);
Texture2D<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  Texture2D<float4> v = t_f;
  uint3 v_1 = (0u).xxx;
  v.GetDimensions(uint(int(1)), v_1[0u], v_1[1u], v_1[2u]);
  uint2 fdims = v_1.xy;
  Texture2D<int4> v_2 = t_i;
  uint3 v_3 = (0u).xxx;
  v_2.GetDimensions(uint(int(1)), v_3[0u], v_3[1u], v_3[2u]);
  uint2 idims = v_3.xy;
  Texture2D<uint4> v_4 = t_u;
  uint3 v_5 = (0u).xxx;
  v_4.GetDimensions(uint(int(1)), v_5[0u], v_5[1u], v_5[2u]);
  uint2 udims = v_5.xy;
}

