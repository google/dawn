SKIP: FAILED

float fwidthFine_f1742d() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_f1742d();
}

