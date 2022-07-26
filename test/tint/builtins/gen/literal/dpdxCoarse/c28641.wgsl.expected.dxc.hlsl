void dpdxCoarse_c28641() {
  float4 res = ddx_coarse((1.0f).xxxx);
}

void fragment_main() {
  dpdxCoarse_c28641();
  return;
}
