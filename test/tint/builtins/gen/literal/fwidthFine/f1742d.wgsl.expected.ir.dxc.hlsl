SKIP: FAILED

float fwidthFine_f1742d() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_f1742d();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthFine_f1742d();
  ^

