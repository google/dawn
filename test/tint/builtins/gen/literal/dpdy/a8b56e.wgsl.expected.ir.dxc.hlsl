SKIP: FAILED

float2 dpdy_a8b56e() {
  float2 res = ddy((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_a8b56e();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdy_a8b56e();
  ^

