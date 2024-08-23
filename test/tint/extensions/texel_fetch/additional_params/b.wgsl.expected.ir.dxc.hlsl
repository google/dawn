SKIP: FAILED

struct f_inputs {
  float4 fbf;
  float4 uv : TEXCOORD0;
  float4 pos : SV_Position;
};


void g(float a, float b, float c) {
}

void f_inner(float4 pos, float4 uv, float4 fbf) {
  g(pos[0u], uv[0u], fbf[0u]);
}

void f(f_inputs inputs) {
  f_inner(float4(inputs.pos.xyz, (1.0f / inputs.pos[3u])), inputs.uv, inputs.fbf);
}

DXC validation failure:
hlsl.hlsl:15:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^


tint executable returned error: exit status 1
