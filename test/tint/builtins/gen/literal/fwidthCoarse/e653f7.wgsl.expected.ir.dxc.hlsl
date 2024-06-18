SKIP: FAILED

float2 fwidthCoarse_e653f7() {
  float2 res = fwidth((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_e653f7();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthCoarse_e653f7();
  ^

