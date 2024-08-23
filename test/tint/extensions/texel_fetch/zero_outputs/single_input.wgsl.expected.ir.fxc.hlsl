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

FXC validation failure:
<scrubbed_path>(13,17-22): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
