SKIP: FAILED

float4 fwidthFine_68f4ef() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_68f4ef();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthFine_68f4ef();
  ^

