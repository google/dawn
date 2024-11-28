
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<uint4> arg_0 : register(u0, space1);
uint4 textureLoad_622278() {
  int3 arg_1 = (int(1)).xxx;
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint3 v_1 = (v - (1u).xxx);
  uint4 res = uint4(arg_0.Load(int4(int3(min(uint3(arg_1), v_1)), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_622278());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_622278());
}

