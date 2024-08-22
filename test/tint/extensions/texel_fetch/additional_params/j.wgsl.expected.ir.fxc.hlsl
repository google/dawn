SKIP: FAILED

struct f_inputs {
  float4 fbf;
  float4 a : TEXCOORD0;
  nointerpolation float4 b : TEXCOORD1;
};


void g(float a, float b, float c) {
}

void f_inner(float4 a, float4 b, float4 fbf) {
  g(a[0u], b[1u], fbf[0u]);
}

void f(f_inputs inputs) {
  f_inner(inputs.a, inputs.b, inputs.fbf);
}

FXC validation failure:
C:\src\dawn\Shader@0x0000026B6F2A5D40(15,17-22): error X3502: 'f': input parameter 'inputs' missing semantics

