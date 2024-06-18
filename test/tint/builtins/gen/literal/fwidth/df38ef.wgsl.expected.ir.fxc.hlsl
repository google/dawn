SKIP: FAILED

float fwidth_df38ef() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = fwidth_df38ef();
}

