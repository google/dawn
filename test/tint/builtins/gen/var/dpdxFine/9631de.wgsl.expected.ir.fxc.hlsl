SKIP: FAILED

float2 dpdxFine_9631de() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_9631de();
}

