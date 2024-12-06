//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<float4> arg_0 : register(u0, space1);
float4 textureLoad_8bf8c2() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  float4 res = float4(arg_0.Load(int2(int(min(1u, (v - 1u))), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_8bf8c2()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<float4> arg_0 : register(u0, space1);
float4 textureLoad_8bf8c2() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  float4 res = float4(arg_0.Load(int2(int(min(1u, (v - 1u))), int(0))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_8bf8c2()));
}

