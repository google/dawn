SKIP: FAILED

float4 fwidthFine_68f4ef() {
  float4 res = fwidth((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_68f4ef();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthFine_68f4ef();
  ^

