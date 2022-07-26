void dpdx_c487fa() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddx(arg_0);
}

void fragment_main() {
  dpdx_c487fa();
  return;
}
