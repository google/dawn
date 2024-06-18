SKIP: FAILED

float4 fwidthCoarse_4e4fc4() {
  float4 res = fwidth((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_4e4fc4();
}

