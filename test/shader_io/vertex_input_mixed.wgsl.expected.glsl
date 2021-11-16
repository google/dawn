#version 310 es
precision mediump float;

struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};
struct VertexInputs1 {
  float loc2;
  vec4 loc3;
};
struct tint_symbol_2 {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  uint vertex_index;
  uint instance_index;
};
struct tint_symbol_3 {
  vec4 value;
};

vec4 tint_symbol_inner(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1) {
  uint foo = (inputs0.vertex_index + instance_index);
  int i = inputs0.loc0;
  uint u = loc1;
  float f = inputs1.loc2;
  vec4 v = inputs1.loc3;
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol_3 tint_symbol(tint_symbol_2 tint_symbol_1) {
  VertexInputs0 tint_symbol_4 = VertexInputs0(tint_symbol_1.vertex_index, tint_symbol_1.loc0);
  VertexInputs1 tint_symbol_5 = VertexInputs1(tint_symbol_1.loc2, tint_symbol_1.loc3);
  vec4 inner_result = tint_symbol_inner(tint_symbol_4, tint_symbol_1.loc1, tint_symbol_1.instance_index, tint_symbol_5);
  tint_symbol_3 wrapper_result = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
in int loc0;
in uint loc1;
in float loc2;
in vec4 loc3;
void main() {
  tint_symbol_2 inputs;
  inputs.loc0 = loc0;
  inputs.loc1 = loc1;
  inputs.loc2 = loc2;
  inputs.loc3 = loc3;
  inputs.vertex_index = uint(gl_VertexID);
  inputs.instance_index = uint(gl_InstanceID);
  tint_symbol_3 outputs;
  outputs = tint_symbol(inputs);
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
}


