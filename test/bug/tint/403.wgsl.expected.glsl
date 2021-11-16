#version 310 es
precision mediump float;


layout (binding = 0) uniform vertexUniformBuffer1_1 {
  mat2 transform1;
} x_20;
layout (binding = 0) uniform vertexUniformBuffer2_1 {
  mat2 transform2;
} x_26;

struct tint_symbol_3 {
  uint tint_symbol_1;
};
struct tint_symbol_4 {
  vec4 value;
};

vec4 tint_symbol_inner(uint tint_symbol_1) {
  vec2 indexable[3] = vec2[3](vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f));
  mat2 x_23 = x_20.transform1;
  mat2 x_28 = x_26.transform2;
  uint x_46 = tint_symbol_1;
  vec2 tint_symbol_5[3] = vec2[3](vec2(-1.0f, 1.0f), vec2(1.0f, 1.0f), vec2(-1.0f, -1.0f));
  indexable = tint_symbol_5;
  vec2 x_51 = indexable[x_46];
  vec2 x_52 = (mat2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])) * x_51);
  return vec4(x_52.x, x_52.y, 0.0f, 1.0f);
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  vec4 inner_result = tint_symbol_inner(tint_symbol_2.tint_symbol_1);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
void main() {
  tint_symbol_3 inputs;
  inputs.tint_symbol_1 = uint(gl_VertexID);
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
}


