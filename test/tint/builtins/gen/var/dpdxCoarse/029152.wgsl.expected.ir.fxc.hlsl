SKIP: FAILED

float dpdxCoarse_029152() {
  float arg_0 = 1.0f;
  float res = ddx_coarse(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxCoarse_029152();
}

