SKIP: FAILED

float2 fwidth_b83ebb() {
  float2 res = fwidth((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidth_b83ebb();
}

