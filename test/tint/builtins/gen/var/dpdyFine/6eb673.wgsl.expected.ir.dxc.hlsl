SKIP: FAILED

float dpdyFine_6eb673() {
  float arg_0 = 1.0f;
  float res = ddy_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyFine_6eb673();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdyFine_6eb673();
  ^

