SKIP: FAILED

struct f_inputs {
  float4 fbf;
};


void g(float a) {
}

void f_inner(float4 fbf) {
  g(fbf[1u]);
}

void f(f_inputs inputs) {
  f_inner(inputs.fbf);
}

DXC validation failure:
hlsl.hlsl:13:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^


tint executable returned error: exit status 1
