SKIP: FAILED

float3 dpdyCoarse_ae1873() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddy_coarse(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyCoarse_ae1873();
}

