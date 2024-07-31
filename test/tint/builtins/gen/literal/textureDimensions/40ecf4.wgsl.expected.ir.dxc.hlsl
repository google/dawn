
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
uint2 textureDimensions_40ecf4() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v[0u], v[1u], v[2u]);
  uint2 res = v.xy;
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, textureDimensions_40ecf4());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, textureDimensions_40ecf4());
}

