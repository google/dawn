SKIP: FAILED

struct In {
  float4 a;
  float4 b;
  int4 fbf;
};

struct f_inputs {
  int4 In_fbf;
  float4 In_a : TEXCOORD0;
  nointerpolation float4 In_b : TEXCOORD1;
};


void g(float a, float b, int c) {
}

void f_inner(In tint_symbol) {
  g(tint_symbol.a[0u], tint_symbol.b[1u], tint_symbol.fbf[0u]);
}

void f(f_inputs inputs) {
  In v = {inputs.In_a, inputs.In_b, inputs.In_fbf};
  f_inner(v);
}

FXC validation failure:
<scrubbed_path>(21,17-22): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
