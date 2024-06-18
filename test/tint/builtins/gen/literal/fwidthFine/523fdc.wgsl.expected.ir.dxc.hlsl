SKIP: FAILED

float3 fwidthFine_523fdc() {
  float3 res = fwidth((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_523fdc();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthFine_523fdc();
  ^

