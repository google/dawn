SKIP: FAILED

struct In {
  float4 pos;
};

struct f_inputs {
  uint4 fbf_0;
  int4 fbf_2;
  float4 uv : TEXCOORD0;
  float4 In_pos : SV_Position;
};


void g(int a, float b, float c, uint d) {
}

void f_inner(int4 fbf_2, In tint_symbol, float4 uv, uint4 fbf_0) {
  g(fbf_2[2u], tint_symbol.pos[0u], uv[0u], fbf_0[1u]);
}

void f(f_inputs inputs) {
  In v = {float4(inputs.In_pos.xyz, (1.0f / inputs.In_pos[3u]))};
  f_inner(inputs.fbf_2, v, inputs.uv, inputs.fbf_0);
}

DXC validation failure:
hlsl.hlsl:20:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^
hlsl.hlsl:20:1: error: Semantic must be defined for all parameters of an entry function or patch constant function


tint executable returned error: exit status 1
