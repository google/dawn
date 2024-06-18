SKIP: FAILED

float fwidthCoarse_159c8a() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_159c8a();
}

