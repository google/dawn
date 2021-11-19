#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0;
vec4 tint_symbol_1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void textureLoad_6273b1() {
  float res = 0.0f;
  vec4 x_17 = vec4(texelFetch(arg_0, ivec2(0, 0), 1).x, 0.0f, 0.0f, 0.0f);
  res = x_17.x;
  return;
}

void tint_symbol_2(vec4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
  return;
}

void vertex_main_1() {
  textureLoad_6273b1();
  tint_symbol_2(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  vec4 tint_symbol_1_1;
};

vertex_main_out vertex_main_inner() {
  vertex_main_1();
  vertex_main_out tint_symbol_4 = vertex_main_out(tint_symbol_1);
  return tint_symbol_4;
}

tint_symbol_3 vertex_main() {
  vertex_main_out inner_result = vertex_main_inner();
  tint_symbol_3 wrapper_result = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.tint_symbol_1_1 = inner_result.tint_symbol_1_1;
  return wrapper_result;
}
void main() {
  tint_symbol_3 outputs;
  outputs = vertex_main();
  gl_Position = outputs.tint_symbol_1_1;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0;

void textureLoad_6273b1() {
  float res = 0.0f;
  vec4 x_17 = vec4(texelFetch(arg_0, ivec2(0, 0), 1).x, 0.0f, 0.0f, 0.0f);
  res = x_17.x;
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  vec4 tint_symbol_1_1;
};

void fragment_main_1() {
  textureLoad_6273b1();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
void main() {
  fragment_main();
}


#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0;

void textureLoad_6273b1() {
  float res = 0.0f;
  vec4 x_17 = vec4(texelFetch(arg_0, ivec2(0, 0), 1).x, 0.0f, 0.0f, 0.0f);
  res = x_17.x;
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  vec4 tint_symbol_1_1;
};

void compute_main_1() {
  textureLoad_6273b1();
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  compute_main_1();
  return;
}
void main() {
  compute_main();
}


