#version 310 es
precision mediump float;

uniform highp writeonly uimage3D arg_0;

void textureStore_f8dead() {
  imageStore(arg_0, ivec3(0, 0, 0), uvec4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  textureStore_f8dead();
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

uniform highp writeonly uimage3D arg_0;

void textureStore_f8dead() {
  imageStore(arg_0, ivec3(0, 0, 0), uvec4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  textureStore_f8dead();
  return;
}
void main() {
  fragment_main();
}


#version 310 es
precision mediump float;

uniform highp writeonly uimage3D arg_0;

void textureStore_f8dead() {
  imageStore(arg_0, ivec3(0, 0, 0), uvec4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  textureStore_f8dead();
  return;
}
void main() {
  compute_main();
}


