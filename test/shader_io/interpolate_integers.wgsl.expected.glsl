SKIP: FAILED

#version 310 es
precision mediump float;

struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};
struct tint_symbol {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};

Interface vert_main_inner() {
  Interface tint_symbol_4 = Interface(0, 0u, ivec4(0, 0, 0, 0), uvec4(0u, 0u, 0u, 0u), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  return tint_symbol_4;
}

struct tint_symbol_2 {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};
struct tint_symbol_3 {
  int value;
};

tint_symbol vert_main() {
  Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = tint_symbol(0, 0u, ivec4(0, 0, 0, 0), uvec4(0u, 0u, 0u, 0u), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.i = inner_result.i;
  wrapper_result.u = inner_result.u;
  wrapper_result.vi = inner_result.vi;
  wrapper_result.vu = inner_result.vu;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
out int i;
out uint u;
out ivec4 vi;
out uvec4 vu;
void main() {
  tint_symbol outputs;
  outputs = vert_main();
  i = outputs.i;
  u = outputs.u;
  vi = outputs.vi;
  vu = outputs.vu;
  gl_Position = outputs.pos;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};
struct tint_symbol {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};
struct tint_symbol_2 {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};
struct tint_symbol_3 {
  int value;
};

int frag_main_inner(Interface inputs) {
  return inputs.i;
}

tint_symbol_3 frag_main(tint_symbol_2 tint_symbol_1) {
  Interface tint_symbol_4 = Interface(tint_symbol_1.i, tint_symbol_1.u, tint_symbol_1.vi, tint_symbol_1.vu, tint_symbol_1.pos);
  int inner_result_1 = frag_main_inner(tint_symbol_4);
  tint_symbol_3 wrapper_result_1 = tint_symbol_3(0);
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
in int i;
in uint u;
in ivec4 vi;
in uvec4 vu;
out int value;
void main() {
  tint_symbol_2 inputs;
  inputs.i = i;
  inputs.u = u;
  inputs.vi = vi;
  inputs.vu = vu;
  inputs.pos = gl_FragCoord;
  tint_symbol_3 outputs;
  outputs = frag_main(inputs);
  value = outputs.value;
}


Error parsing GLSL shader:
ERROR: 0:40: 'int' : must be qualified as flat in
ERROR: 0:40: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



