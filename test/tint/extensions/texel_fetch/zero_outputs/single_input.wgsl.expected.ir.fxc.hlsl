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
C:\src\dawn\Shader@0x00000232C264AD50(13,17-22): error X3502: 'f': input parameter 'inputs' missing semantics

