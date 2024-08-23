SKIP: FAILED

struct f_inputs {
  float4 fbf_1;
  float4 fbf_3;
};


void g(float a, float b) {
}

void f_inner(float4 fbf_1, float4 fbf_3) {
  g(fbf_1[0u], fbf_3[1u]);
}

void f(f_inputs inputs) {
  f_inner(inputs.fbf_1, inputs.fbf_3);
}

DXC validation failure:
hlsl.hlsl:14:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^
hlsl.hlsl:14:1: error: Semantic must be defined for all parameters of an entry function or patch constant function


tint executable returned error: exit status 1
