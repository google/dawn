SKIP: FAILED

#version 310 es

uint tint_pack4x8unorm(vec4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}


void pack4x8unorm_95c456() {
  uint res = tint_pack4x8unorm(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

vec4 vertex_main() {
  pack4x8unorm_95c456();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'uint4' : undeclared identifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uint tint_pack4x8unorm(vec4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}


void pack4x8unorm_95c456() {
  uint res = tint_pack4x8unorm(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  pack4x8unorm_95c456();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:5: 'uint4' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uint tint_pack4x8unorm(vec4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}


void pack4x8unorm_95c456() {
  uint res = tint_pack4x8unorm(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void compute_main() {
  pack4x8unorm_95c456();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'uint4' : undeclared identifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



