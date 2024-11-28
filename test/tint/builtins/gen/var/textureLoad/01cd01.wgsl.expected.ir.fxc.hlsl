
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);
int4 textureLoad_01cd01() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(arg_2, (v.z - 1u));
  uint3 v_2 = (0u).xxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z);
  int2 v_3 = int2(min(arg_1, (v_2.xy - (1u).xx)));
  int4 res = int4(arg_0.Load(int4(v_3, int(v_1), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_01cd01()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_01cd01()));
}

