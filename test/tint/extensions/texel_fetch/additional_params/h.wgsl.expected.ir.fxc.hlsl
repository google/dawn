SKIP: FAILED

struct FBF {
  float4 c1;
  int4 c3;
};

struct f_inputs {
  float4 FBF_c1;
  int4 FBF_c3;
  precise float4 pos : SV_Position;
};


void g(float a, float b, int c) {
}

void f_inner(float4 pos, FBF fbf) {
  g(fbf.c1[0u], pos[1u], fbf.c3[2u]);
}

void f(f_inputs inputs) {
  float4 v = float4(inputs.pos.xyz, (1.0f / inputs.pos[3u]));
  FBF v_1 = {inputs.FBF_c1, inputs.FBF_c3};
  f_inner(v, v_1);
}

FXC validation failure:
<scrubbed_path>(20,17-22): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
