SKIP: FAILED

float dpdx_e263de() {
  float res = ddx(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_e263de();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdx_e263de();
  ^

