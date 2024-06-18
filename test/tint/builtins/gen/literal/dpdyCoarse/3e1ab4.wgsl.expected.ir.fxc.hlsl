SKIP: FAILED

float2 dpdyCoarse_3e1ab4() {
  float2 res = ddy_coarse((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyCoarse_3e1ab4();
}

