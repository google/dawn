SKIP: FAILED

struct In {
  float4 uv;
};

struct f_inputs {
  float4 fbf;
  float4 In_uv : TEXCOORD0;
  float4 pos : SV_Position;
};


void g(float a, float b, float c) {
}

void f_inner(float4 pos, float4 fbf, In tint_symbol) {
  g(pos[0u], fbf[0u], tint_symbol.uv[0u]);
}

void f(f_inputs inputs) {
  float4 v = float4(inputs.pos.xyz, (1.0f / inputs.pos[3u]));
  In v_1 = {inputs.In_uv};
  f_inner(v, inputs.fbf, v_1);
}

DXC validation failure:
hlsl.hlsl:19:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^


tint executable returned error: exit status 1
