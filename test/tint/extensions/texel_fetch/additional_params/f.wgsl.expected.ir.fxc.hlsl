SKIP: FAILED

struct In {
  float4 pos;
};

struct f_inputs {
  float4 fbf;
  precise float4 In_pos : SV_Position;
};


void g(float a, float b) {
}

void f_inner(In tint_symbol, float4 fbf) {
  g(tint_symbol.pos[0u], fbf[1u]);
}

void f(f_inputs inputs) {
  In v = {float4(inputs.In_pos.xyz, (1.0f / inputs.In_pos[3u]))};
  f_inner(v, inputs.fbf);
}

FXC validation failure:
<scrubbed_path>(18,17-22): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
