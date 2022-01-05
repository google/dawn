#version 310 es
precision mediump float;

vec2 tint_radians(vec2 param_0) {
  return param_0 * 0.017453292519943295474;
}


void radians_61687a() {
  vec2 res = tint_radians(vec2(0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  radians_61687a();
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

vec2 tint_radians(vec2 param_0) {
  return param_0 * 0.017453292519943295474;
}


void radians_61687a() {
  vec2 res = tint_radians(vec2(0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  radians_61687a();
  return;
}
void main() {
  fragment_main();
}


#version 310 es
precision mediump float;

vec2 tint_radians(vec2 param_0) {
  return param_0 * 0.017453292519943295474;
}


void radians_61687a() {
  vec2 res = tint_radians(vec2(0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  radians_61687a();
  return;
}
void main() {
  compute_main();
}


