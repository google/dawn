SKIP: FAILED

float2 dpdxCoarse_9581cf() {
  float2 res = ddx_coarse((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxCoarse_9581cf();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxCoarse_9581cf();
  ^

