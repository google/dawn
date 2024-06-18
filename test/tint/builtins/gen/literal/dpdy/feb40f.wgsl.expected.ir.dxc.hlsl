SKIP: FAILED

float3 dpdy_feb40f() {
  float3 res = ddy((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_feb40f();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdy_feb40f();
  ^

