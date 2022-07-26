void dpdxCoarse_9581cf() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx_coarse(arg_0);
}

void fragment_main() {
  dpdxCoarse_9581cf();
  return;
}
