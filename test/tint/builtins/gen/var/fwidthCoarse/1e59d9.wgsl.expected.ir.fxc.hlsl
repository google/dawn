SKIP: FAILED

float3 fwidthCoarse_1e59d9() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_1e59d9();
}

