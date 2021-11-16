#version 310 es
precision mediump float;

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

void dot_e994c7() {
  uint res = tint_int_dot(uvec4(0u, 0u, 0u, 0u), uvec4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  dot_e994c7();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  vec4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = tint_symbol(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
void main() {
  tint_symbol outputs;
  outputs = vertex_main();
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

void dot_e994c7() {
  uint res = tint_int_dot(uvec4(0u, 0u, 0u, 0u), uvec4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  dot_e994c7();
  return;
}
void main() {
  fragment_main();
}


#version 310 es
precision mediump float;

uint tint_int_dot(uvec4 a, uvec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

void dot_e994c7() {
  uint res = tint_int_dot(uvec4(0u, 0u, 0u, 0u), uvec4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  dot_e994c7();
  return;
}
void main() {
  compute_main();
}


