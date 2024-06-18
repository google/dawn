SKIP: FAILED

float2 fwidthCoarse_e653f7() {
  float2 res = fwidth((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_e653f7();
}

