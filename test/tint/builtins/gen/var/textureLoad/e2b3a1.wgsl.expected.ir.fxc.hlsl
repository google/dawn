
RWByteAddressBuffer prevent_dce : register(u0);
RWTexture2DArray<int4> arg_0 : register(u0, space1);
int4 textureLoad_e2b3a1() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int v = arg_2;
  int2 v_1 = int2(arg_1);
  int4 res = int4(arg_0.Load(int4(v_1, int(v), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_e2b3a1()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_e2b3a1()));
}

