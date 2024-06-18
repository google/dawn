SKIP: FAILED

float dpdy_7f8d84() {
  float arg_0 = 1.0f;
  float res = ddy(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_7f8d84();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdy_7f8d84();
  ^

