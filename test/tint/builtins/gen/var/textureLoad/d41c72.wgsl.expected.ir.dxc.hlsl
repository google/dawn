
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture3D<int4> arg_0 : register(u0, space1);
int4 textureLoad_d41c72() {
  int3 arg_1 = (int(1)).xxx;
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint3 v_1 = (v - (1u).xxx);
  int4 res = int4(arg_0.Load(int4(int3(min(uint3(arg_1), v_1)), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_d41c72()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_d41c72()));
}

