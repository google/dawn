#version 310 es

struct tint_symbol_3 {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_4_block_ubo {
  tint_symbol_3 inner;
} tint_symbol_4;

vec4 tint_symbol_1 = vec4(0.0f);
void textureNumSamples_a3c8a0() {
  int res = 0;
  res = int(tint_symbol_4.inner.texture_builtin_value_0);
  return;
}

void tint_symbol_2(vec4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
  return;
}

void vertex_main_1() {
  textureNumSamples_a3c8a0();
  tint_symbol_2(vec4(0.0f));
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

vertex_main_out vertex_main() {
  vertex_main_1();
  vertex_main_out tint_symbol_5 = vertex_main_out(tint_symbol_1);
  return tint_symbol_5;
}

void main() {
  gl_PointSize = 1.0;
  vertex_main_out inner_result = vertex_main();
  gl_Position = inner_result.tint_symbol_1_1;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;
precision highp int;

struct tint_symbol_3 {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_4_block_ubo {
  tint_symbol_3 inner;
} tint_symbol_4;

void textureNumSamples_a3c8a0() {
  int res = 0;
  res = int(tint_symbol_4.inner.texture_builtin_value_0);
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

void fragment_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

void fragment_main() {
  fragment_main_1();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct tint_symbol_3 {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_4_block_ubo {
  tint_symbol_3 inner;
} tint_symbol_4;

void textureNumSamples_a3c8a0() {
  int res = 0;
  res = int(tint_symbol_4.inner.texture_builtin_value_0);
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

void compute_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

void compute_main() {
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
