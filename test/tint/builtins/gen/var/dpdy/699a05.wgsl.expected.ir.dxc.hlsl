SKIP: FAILED

float4 dpdy_699a05() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddy(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdy_699a05();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdy_699a05();
  ^

