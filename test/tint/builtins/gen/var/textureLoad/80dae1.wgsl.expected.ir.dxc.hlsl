//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<int4> arg_0 : register(u0, space1);
int4 textureLoad_80dae1() {
  uint arg_1 = 1u;
  uint v = 0u;
  arg_0.GetDimensions(v);
  int4 res = int4(arg_0.Load(int2(int(min(arg_1, (v - 1u))), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_80dae1()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<int4> arg_0 : register(u0, space1);
int4 textureLoad_80dae1() {
  uint arg_1 = 1u;
  uint v = 0u;
  arg_0.GetDimensions(v);
  int4 res = int4(arg_0.Load(int2(int(min(arg_1, (v - 1u))), int(0))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_80dae1()));
}

