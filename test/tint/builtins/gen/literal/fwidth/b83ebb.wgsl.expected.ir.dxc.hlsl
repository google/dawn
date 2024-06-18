SKIP: FAILED

float2 fwidth_b83ebb() {
  float2 res = fwidth((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidth_b83ebb();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidth_b83ebb();
  ^

