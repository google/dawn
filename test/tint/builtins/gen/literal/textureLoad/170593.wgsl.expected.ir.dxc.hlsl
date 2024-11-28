
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_170593() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  uint2 v_1 = (v - (1u).xx);
  uint4 res = uint4(arg_0.Load(int3(int2(min(uint2((int(1)).xx), v_1)), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_170593());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_170593());
}

