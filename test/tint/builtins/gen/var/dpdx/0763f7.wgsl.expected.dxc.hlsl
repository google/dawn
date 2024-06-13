float3 dpdx_0763f7() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx(arg_0);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store3(0u, asuint(dpdx_0763f7()));
  return;
}
