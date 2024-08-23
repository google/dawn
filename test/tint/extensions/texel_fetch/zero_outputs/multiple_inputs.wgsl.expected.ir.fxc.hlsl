SKIP: FAILED

struct f_inputs {
  float4 fbf_1;
  float4 fbf_3;
};


void g(float a, float b) {
}

void f_inner(float4 fbf_1, float4 fbf_3) {
  g(fbf_1[0u], fbf_3[1u]);
}

void f(f_inputs inputs) {
  f_inner(inputs.fbf_1, inputs.fbf_3);
}

FXC validation failure:
<scrubbed_path>(14,17-22): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
