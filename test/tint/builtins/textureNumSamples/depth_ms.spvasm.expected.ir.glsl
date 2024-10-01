#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

uniform highp sampler2DMS arg_0;
vec4 tint_symbol_1 = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  TintTextureUniformData tint_symbol_3;
} v;
void textureNumSamples_a3c8a0() {
  int res = 0;
  res = int(v.tint_symbol_3.tint_builtin_value_0);
}
void tint_symbol_2(vec4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
}
void vertex_main_1() {
  textureNumSamples_a3c8a0();
  tint_symbol_2(vec4(0.0f));
}
vertex_main_out vertex_main_inner() {
  vertex_main_1();
  return vertex_main_out(tint_symbol_1);
}
void main() {
  gl_Position = vertex_main_inner().tint_symbol_1_1;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

uniform highp sampler2DMS arg_0;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData tint_symbol;
} v;
void textureNumSamples_a3c8a0() {
  int res = 0;
  res = int(v.tint_symbol.tint_builtin_value_0);
}
void fragment_main_1() {
  textureNumSamples_a3c8a0();
}
void main() {
  fragment_main_1();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

uniform highp sampler2DMS arg_0;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData tint_symbol;
} v;
void textureNumSamples_a3c8a0() {
  int res = 0;
  res = int(v.tint_symbol.tint_builtin_value_0);
}
void compute_main_1() {
  textureNumSamples_a3c8a0();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
