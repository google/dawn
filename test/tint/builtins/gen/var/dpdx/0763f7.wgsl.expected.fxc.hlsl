void dpdx_0763f7() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx(arg_0);
}

void fragment_main() {
  dpdx_0763f7();
  return;
}
