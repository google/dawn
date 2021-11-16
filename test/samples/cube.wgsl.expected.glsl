#version 310 es
precision mediump float;


layout (binding = 0) uniform Uniforms_1 {
  mat4 modelViewProjectionMatrix;
} uniforms;

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};
struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};
struct tint_symbol_3 {
  vec4 cur_position;
  vec4 color;
};
struct tint_symbol_4 {
  vec4 vtxFragColor;
  vec4 Position;
};

VertexOutput vtx_main_inner(VertexInput tint_symbol) {
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_1.Position = (uniforms.modelViewProjectionMatrix * tint_symbol.cur_position);
  tint_symbol_1.vtxFragColor = tint_symbol.color;
  return tint_symbol_1;
}

struct tint_symbol_6 {
  vec4 fragColor;
};
struct tint_symbol_7 {
  vec4 value;
};

tint_symbol_4 vtx_main(tint_symbol_3 tint_symbol_2) {
  VertexInput tint_symbol_8 = VertexInput(tint_symbol_2.cur_position, tint_symbol_2.color);
  VertexOutput inner_result = vtx_main_inner(tint_symbol_8);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.vtxFragColor = inner_result.vtxFragColor;
  wrapper_result.Position = inner_result.Position;
  return wrapper_result;
}
in vec4 cur_position;
in vec4 color;
out vec4 vtxFragColor;
void main() {
  tint_symbol_3 inputs;
  inputs.cur_position = cur_position;
  inputs.color = color;
  tint_symbol_4 outputs;
  outputs = vtx_main(inputs);
  vtxFragColor = outputs.vtxFragColor;
  gl_Position = outputs.Position;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};
struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};
struct tint_symbol_3 {
  vec4 cur_position;
  vec4 color;
};
struct tint_symbol_4 {
  vec4 vtxFragColor;
  vec4 Position;
};
struct tint_symbol_6 {
  vec4 fragColor;
};
struct tint_symbol_7 {
  vec4 value;
};

vec4 frag_main_inner(vec4 fragColor) {
  return fragColor;
}

tint_symbol_7 frag_main(tint_symbol_6 tint_symbol_5) {
  vec4 inner_result_1 = frag_main_inner(tint_symbol_5.fragColor);
  tint_symbol_7 wrapper_result_1 = tint_symbol_7(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
in vec4 fragColor;
out vec4 value;
void main() {
  tint_symbol_6 inputs;
  inputs.fragColor = fragColor;
  tint_symbol_7 outputs;
  outputs = frag_main(inputs);
  value = outputs.value;
}


