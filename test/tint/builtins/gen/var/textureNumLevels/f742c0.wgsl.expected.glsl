SKIP: FAILED

#version 310 es

uniform highp isampler2D arg_0_1;
void textureNumLevels_f742c0() {
  uint res = uint(textureQueryLevels(arg_0_1));
}

vec4 vertex_main() {
  textureNumLevels_f742c0();
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
Error parsing GLSL shader:
ERROR: 0:5: 'textureQueryLevels' : no matching overloaded function found
ERROR: 0:5: '' : compilation terminated
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp isampler2D arg_0_1;
void textureNumLevels_f742c0() {
  uint res = uint(textureQueryLevels(arg_0_1));
}

void fragment_main() {
  textureNumLevels_f742c0();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'textureQueryLevels' : no matching overloaded function found
ERROR: 0:6: '' : compilation terminated
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp isampler2D arg_0_1;
void textureNumLevels_f742c0() {
  uint res = uint(textureQueryLevels(arg_0_1));
}

void compute_main() {
  textureNumLevels_f742c0();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:5: 'textureQueryLevels' : no matching overloaded function found
ERROR: 0:5: '' : compilation terminated
ERROR: 2 compilation errors.  No code generated.



