SKIP: FAILED

float4 dpdy_699a05() {
  float4 res = ddy((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_699a05();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdy_699a05();
  ^

