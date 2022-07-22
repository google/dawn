void fwidth_b83ebb() {
  float2 res = fwidth((0.0f).xx);
}

void fragment_main() {
  fwidth_b83ebb();
  return;
}
