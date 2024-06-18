SKIP: FAILED

float fwidth_df38ef() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidth_df38ef();
}

