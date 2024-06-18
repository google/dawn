SKIP: FAILED

float dpdy_7f8d84() {
  float res = ddy(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_7f8d84();
}

