SKIP: FAILED

#version 310 es
precision mediump float;

vec2 tint_unpack2x16float(uint param_0) {
  uint i = param_0;
  return f16tof32(uint2(i & 0xffff, i >> 16));
}


void unpack2x16float_32a5cf() {
  vec2 res = tint_unpack2x16float(1u);
}

vec4 vertex_main() {
  unpack2x16float_32a5cf();
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
ERROR: 0:6: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp highp uint' and a right operand of type ' const int' (or there is no acceptable conversion)
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec2 tint_unpack2x16float(uint param_0) {
  uint i = param_0;
  return f16tof32(uint2(i & 0xffff, i >> 16));
}


void unpack2x16float_32a5cf() {
  vec2 res = tint_unpack2x16float(1u);
}

void fragment_main() {
  unpack2x16float_32a5cf();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp mediump uint' and a right operand of type ' const int' (or there is no acceptable conversion)
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec2 tint_unpack2x16float(uint param_0) {
  uint i = param_0;
  return f16tof32(uint2(i & 0xffff, i >> 16));
}


void unpack2x16float_32a5cf() {
  vec2 res = tint_unpack2x16float(1u);
}

void compute_main() {
  unpack2x16float_32a5cf();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp highp uint' and a right operand of type ' const int' (or there is no acceptable conversion)
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



