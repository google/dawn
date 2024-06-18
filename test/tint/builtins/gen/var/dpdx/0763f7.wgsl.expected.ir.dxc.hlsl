SKIP: FAILED

float3 dpdx_0763f7() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_0763f7();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdx_0763f7();
  ^

