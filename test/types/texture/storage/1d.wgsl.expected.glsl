SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp writeonly image1D t_rgba8unorm;
uniform highp writeonly image1D t_rgba8snorm;
uniform highp writeonly uimage1D t_rgba8uint;
uniform highp writeonly iimage1D t_rgba8sint;
uniform highp writeonly uimage1D t_rgba16uint;
uniform highp writeonly iimage1D t_rgba16sint;
uniform highp writeonly image1D t_rgba16float;
uniform highp writeonly uimage1D t_r32uint;
uniform highp writeonly iimage1D t_r32sint;
uniform highp writeonly image1D t_r32float;
uniform highp writeonly uimage1D t_rg32uint;
uniform highp writeonly iimage1D t_rg32sint;
uniform highp writeonly image1D t_rg32float;
uniform highp writeonly uimage1D t_rgba32uint;
uniform highp writeonly iimage1D t_rgba32sint;
uniform highp writeonly image1D t_rgba32float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:4: 'image1D' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



