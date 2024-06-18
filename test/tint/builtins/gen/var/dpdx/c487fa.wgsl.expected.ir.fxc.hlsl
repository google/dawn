SKIP: FAILED

float4 dpdx_c487fa() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddx(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_c487fa();
}

