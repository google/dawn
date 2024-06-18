SKIP: FAILED

float3 fwidthFine_523fdc() {
  float3 res = fwidth((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_523fdc();
}

