SKIP: FAILED

float2 fwidth_b83ebb() {
  float2 arg_0 = (1.0f).xx;
  float2 res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidth_b83ebb();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidth_b83ebb();
  ^

