SKIP: FAILED

#version 310 es
precision mediump float;

uint tint_pack4x8snorm(vec4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}


void pack4x8snorm_4d22e7() {
  uint res = tint_pack4x8snorm(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  pack4x8snorm_4d22e7();
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


Error parsing GLSL shader:
ERROR: 0:5: 'int4' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uint tint_pack4x8snorm(vec4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}


void pack4x8snorm_4d22e7() {
  uint res = tint_pack4x8snorm(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  pack4x8snorm_4d22e7();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'int4' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uint tint_pack4x8snorm(vec4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}


void pack4x8snorm_4d22e7() {
  uint res = tint_pack4x8snorm(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  pack4x8snorm_4d22e7();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'int4' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



