void dpdxCoarse_c28641() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddx_coarse(arg_0);
}

void fragment_main() {
  dpdxCoarse_c28641();
  return;
}
