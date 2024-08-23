SKIP: FAILED

struct f_outputs {
  float4 tint_symbol : SV_Target0;
};

struct f_inputs {
  float4 fbf;
};


float4 f_inner(float4 fbf) {
  return fbf;
}

f_outputs f(f_inputs inputs) {
  f_outputs v = {f_inner(inputs.fbf)};
  return v;
}

DXC validation failure:
hlsl.hlsl:14:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
f_outputs f(f_inputs inputs) {
^


tint executable returned error: exit status 1
