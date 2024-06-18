SKIP: FAILED

float fwidthCoarse_159c8a() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = fwidthCoarse_159c8a();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidthCoarse_159c8a();
  ^

