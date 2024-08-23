SKIP: FAILED

struct f_inputs {
  float4 fbf;
  float4 a : TEXCOORD0;
  nointerpolation float4 b : TEXCOORD1;
};


void g(float a, float b, float c) {
}

void f_inner(float4 a, float4 b, float4 fbf) {
  g(a[0u], b[1u], fbf[0u]);
}

void f(f_inputs inputs) {
  f_inner(inputs.a, inputs.b, inputs.fbf);
}

DXC validation failure:
hlsl.hlsl:15:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^


tint executable returned error: exit status 1
