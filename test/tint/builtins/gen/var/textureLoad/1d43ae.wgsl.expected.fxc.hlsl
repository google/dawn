//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<int4> arg_0 : register(u0, space1);
int4 textureLoad_1d43ae() {
  int arg_1 = int(1);
  int4 res = arg_0.Load(int2(arg_1, int(0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_1d43ae()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<int4> arg_0 : register(u0, space1);
int4 textureLoad_1d43ae() {
  int arg_1 = int(1);
  int4 res = arg_0.Load(int2(arg_1, int(0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_1d43ae()));
}

