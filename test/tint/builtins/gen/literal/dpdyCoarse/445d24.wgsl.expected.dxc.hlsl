void dpdyCoarse_445d24() {
  float4 res = ddy_coarse((1.0f).xxxx);
}

void fragment_main() {
  dpdyCoarse_445d24();
  return;
}
