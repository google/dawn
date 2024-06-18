SKIP: FAILED

float fwidthCoarse_159c8a() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_159c8a();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthCoarse_159c8a();
  ^

