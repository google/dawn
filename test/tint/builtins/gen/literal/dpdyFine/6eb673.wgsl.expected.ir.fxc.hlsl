SKIP: FAILED

float dpdyFine_6eb673() {
  float res = ddy_fine(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyFine_6eb673();
}

