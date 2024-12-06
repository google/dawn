//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<int4> arg_0 : register(u0, space1);
int4 textureLoad_d72de9() {
  int2 arg_1 = (int(1)).xx;
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  uint2 v_1 = (v - (1u).xx);
  int4 res = int4(arg_0.Load(int3(int2(min(uint2(arg_1), v_1)), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_d72de9()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<int4> arg_0 : register(u0, space1);
int4 textureLoad_d72de9() {
  int2 arg_1 = (int(1)).xx;
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  uint2 v_1 = (v - (1u).xx);
  int4 res = int4(arg_0.Load(int3(int2(min(uint2(arg_1), v_1)), int(0))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_d72de9()));
}

