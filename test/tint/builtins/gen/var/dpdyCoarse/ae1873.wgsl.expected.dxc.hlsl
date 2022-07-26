void dpdyCoarse_ae1873() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddy_coarse(arg_0);
}

void fragment_main() {
  dpdyCoarse_ae1873();
  return;
}
