
Texture1D<float4> t_f : register(t0);
Texture1D<int4> t_i : register(t1);
Texture1D<uint4> t_u : register(t2);
[numthreads(1, 1, 1)]
void main() {
  uint2 v = (0u).xx;
  t_f.GetDimensions(0u, v.x, v.y);
  uint2 v_1 = (0u).xx;
  t_f.GetDimensions(uint(min(uint(int(1)), (v.y - 1u))), v_1.x, v_1.y);
  uint fdims = v_1.x;
  uint2 v_2 = (0u).xx;
  t_i.GetDimensions(0u, v_2.x, v_2.y);
  uint2 v_3 = (0u).xx;
  t_i.GetDimensions(uint(min(uint(int(1)), (v_2.y - 1u))), v_3.x, v_3.y);
  uint idims = v_3.x;
  uint2 v_4 = (0u).xx;
  t_u.GetDimensions(0u, v_4.x, v_4.y);
  uint2 v_5 = (0u).xx;
  t_u.GetDimensions(uint(min(uint(int(1)), (v_4.y - 1u))), v_5.x, v_5.y);
  uint udims = v_5.x;
}

