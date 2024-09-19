
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);
int4 textureLoad_32a7b8() {
  RWTexture2DArray<int4> v = arg_0;
  int2 v_1 = int2((1u).xx);
  int4 res = int4(v.Load(int4(v_1, int(int(1)), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_32a7b8()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_32a7b8()));
}

