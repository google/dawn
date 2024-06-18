SKIP: FAILED

float fwidthCoarse_159c8a() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_159c8a();
}

