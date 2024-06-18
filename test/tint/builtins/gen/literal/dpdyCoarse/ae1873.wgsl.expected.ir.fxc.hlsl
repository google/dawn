SKIP: FAILED

float3 dpdyCoarse_ae1873() {
  float3 res = ddy_coarse((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyCoarse_ae1873();
}

