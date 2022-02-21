void dpdxCoarse_c28641() {
  float4 res = ddx_coarse(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdxCoarse_c28641();
  return;
}
