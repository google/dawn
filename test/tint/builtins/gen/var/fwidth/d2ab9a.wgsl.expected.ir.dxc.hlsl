SKIP: FAILED

float4 fwidth_d2ab9a() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidth_d2ab9a();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidth_d2ab9a();
  ^

