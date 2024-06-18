SKIP: FAILED

float dpdy_7f8d84() {
  float res = ddy(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_7f8d84();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdy_7f8d84();
  ^

