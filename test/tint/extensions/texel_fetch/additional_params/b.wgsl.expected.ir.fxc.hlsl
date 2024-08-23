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

FXC validation failure:
<scrubbed_path>(15,17-22): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
