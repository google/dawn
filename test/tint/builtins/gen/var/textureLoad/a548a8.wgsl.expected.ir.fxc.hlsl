
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture1D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_a548a8() {
  int arg_1 = int(1);
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint v_1 = (v - 1u);
  uint4 res = uint4(arg_0.Load(int2(int(min(uint(arg_1), v_1)), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_a548a8());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_a548a8());
}

