void dpdxCoarse_9581cf() {
  float2 res = ddx_coarse((1.0f).xx);
}

void fragment_main() {
  dpdxCoarse_9581cf();
  return;
}
