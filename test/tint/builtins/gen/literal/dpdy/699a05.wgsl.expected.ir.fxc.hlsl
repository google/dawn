SKIP: FAILED

float4 dpdy_699a05() {
  float4 res = ddy((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_699a05();
}

