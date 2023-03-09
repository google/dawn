#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};


void frexp_bee870() {
  frexp_result_f32 res = frexp_result_f32(0.5f, 1);
}

vec4 vertex_main() {
  frexp_bee870();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

struct frexp_result_f32 {
  float fract;
  int exp;
};


void frexp_bee870() {
  frexp_result_f32 res = frexp_result_f32(0.5f, 1);
}

void fragment_main() {
  frexp_bee870();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};


void frexp_bee870() {
  frexp_result_f32 res = frexp_result_f32(0.5f, 1);
}

void compute_main() {
  frexp_bee870();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
