#version 310 es
precision mediump float;

struct tint_symbol_1 {
  uint VertexIndex;
};
struct tint_symbol_2 {
  vec4 value;
};

vec4 vtx_main_inner(uint VertexIndex) {
  vec2 pos[3] = vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
  return vec4(pos[VertexIndex], 0.0f, 1.0f);
}

struct tint_symbol_3 {
  vec4 value;
};

tint_symbol_2 vtx_main(tint_symbol_1 tint_symbol) {
  vec4 inner_result = vtx_main_inner(tint_symbol.VertexIndex);
  tint_symbol_2 wrapper_result = tint_symbol_2(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
void main() {
  tint_symbol_1 inputs;
  inputs.VertexIndex = uint(gl_VertexID);
  tint_symbol_2 outputs;
  outputs = vtx_main(inputs);
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

struct tint_symbol_1 {
  uint VertexIndex;
};
struct tint_symbol_2 {
  vec4 value;
};
struct tint_symbol_3 {
  vec4 value;
};

vec4 frag_main_inner() {
  return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

tint_symbol_3 frag_main() {
  vec4 inner_result_1 = frag_main_inner();
  tint_symbol_3 wrapper_result_1 = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
out vec4 value;
void main() {
  tint_symbol_3 outputs;
  outputs = frag_main();
  value = outputs.value;
}


