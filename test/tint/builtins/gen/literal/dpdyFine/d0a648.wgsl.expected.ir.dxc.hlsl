SKIP: FAILED

float4 dpdyFine_d0a648() {
  float4 res = ddy_fine((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyFine_d0a648();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdyFine_d0a648();
  ^

