RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdx_0763f7() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdx_0763f7();
  return;
}
