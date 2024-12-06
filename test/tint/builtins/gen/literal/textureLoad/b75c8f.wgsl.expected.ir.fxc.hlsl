//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<int4> arg_0 : register(u0, space1);
int4 textureLoad_b75c8f() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  int4 res = int4(arg_0.Load(int3(int2(min((1u).xx, (v - (1u).xx))), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b75c8f()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<int4> arg_0 : register(u0, space1);
int4 textureLoad_b75c8f() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  int4 res = int4(arg_0.Load(int3(int2(min((1u).xx, (v - (1u).xx))), int(0))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_b75c8f()));
}

