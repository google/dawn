SKIP: FAILED

float dpdxFine_f401a2() {
  float res = ddx_fine(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_f401a2();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxFine_f401a2();
  ^

