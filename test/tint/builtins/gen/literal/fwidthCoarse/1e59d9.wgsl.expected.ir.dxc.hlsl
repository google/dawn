SKIP: FAILED

float3 fwidthCoarse_1e59d9() {
  float3 res = fwidth((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_1e59d9();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthCoarse_1e59d9();
  ^

