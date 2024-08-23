SKIP: FAILED

struct f_inputs {
  float4 fbf;
  float4 pos : SV_Position;
};


void g(float a, float b) {
}

void f_inner(float4 fbf, float4 pos) {
  g(fbf[3u], pos[0u]);
}

void f(f_inputs inputs) {
  f_inner(inputs.fbf, float4(inputs.pos.xyz, (1.0f / inputs.pos[3u])));
}

DXC validation failure:
hlsl.hlsl:14:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^


tint executable returned error: exit status 1
