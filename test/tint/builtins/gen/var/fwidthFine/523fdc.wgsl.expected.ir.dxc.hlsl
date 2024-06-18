SKIP: FAILED

float3 fwidthFine_523fdc() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthFine_523fdc();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthFine_523fdc();
  ^

