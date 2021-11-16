SKIP: FAILED

#version 310 es
precision mediump float;

struct FragIn {
  float a;
  uint mask;
};
struct tint_symbol_3 {
  float a;
  float b;
  uint mask;
};
struct tint_symbol_4 {
  float a;
  uint mask;
};

FragIn tint_symbol_inner(FragIn tint_symbol_1, float b) {
  if ((tint_symbol_1.mask == 0u)) {
    return tint_symbol_1;
  }
  FragIn tint_symbol_5 = FragIn(b, 1u);
  return tint_symbol_5;
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  FragIn tint_symbol_6 = FragIn(tint_symbol_2.a, tint_symbol_2.mask);
  FragIn inner_result = tint_symbol_inner(tint_symbol_6, tint_symbol_2.b);
  tint_symbol_4 wrapper_result = tint_symbol_4(0.0f, 0u);
  wrapper_result.a = inner_result.a;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
in float a;
in float b;
out float a;
void main() {
  tint_symbol_3 inputs;
  inputs.a = a;
  inputs.b = b;
  inputs.mask = uint(gl_SampleMask);
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  a = outputs.a;
  gl_SampleMask = outputs.mask;
}


Error parsing GLSL shader:
ERROR: 0:36: 'a' : redefinition 
ERROR: 1 compilation errors.  No code generated.



