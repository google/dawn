SKIP: FAILED

float2 dpdxFine_9631de() {
  float2 res = ddx_fine((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_9631de();
}

