SKIP: FAILED

float dpdxFine_f401a2() {
  float arg_0 = 1.0f;
  float res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_f401a2();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxFine_f401a2();
  ^

