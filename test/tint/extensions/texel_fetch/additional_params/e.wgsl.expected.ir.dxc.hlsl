SKIP: FAILED

struct In {
  int4 fbf;
  float4 pos;
};

struct f_inputs {
  int4 In_fbf;
  float4 In_pos : SV_Position;
};


void g(int a, float b) {
}

void f_inner(In tint_symbol) {
  g(tint_symbol.fbf[3u], tint_symbol.pos[0u]);
}

void f(f_inputs inputs) {
  In v = {inputs.In_fbf, float4(inputs.In_pos.xyz, (1.0f / inputs.In_pos[3u]))};
  f_inner(v);
}

DXC validation failure:
hlsl.hlsl:19:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
void f(f_inputs inputs) {
^


tint executable returned error: exit status 1
