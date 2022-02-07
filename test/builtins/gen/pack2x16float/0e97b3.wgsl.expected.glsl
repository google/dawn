SKIP: FAILED

#version 310 es

uint tint_pack2x16float(vec2 param_0) {
  uint2 i = f32tof16(param_0);
  return i.x | (i.y << 16);
}


void pack2x16float_0e97b3() {
  uint res = tint_pack2x16float(vec2(0.0f, 0.0f));
}

vec4 vertex_main() {
  pack2x16float_0e97b3();
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
ERROR: 0:4: 'uint2' : undeclared identifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uint tint_pack2x16float(vec2 param_0) {
  uint2 i = f32tof16(param_0);
  return i.x | (i.y << 16);
}


void pack2x16float_0e97b3() {
  uint res = tint_pack2x16float(vec2(0.0f, 0.0f));
}

void fragment_main() {
  pack2x16float_0e97b3();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:5: 'uint2' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uint tint_pack2x16float(vec2 param_0) {
  uint2 i = f32tof16(param_0);
  return i.x | (i.y << 16);
}


void pack2x16float_0e97b3() {
  uint res = tint_pack2x16float(vec2(0.0f, 0.0f));
}

void compute_main() {
  pack2x16float_0e97b3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'uint2' : undeclared identifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



