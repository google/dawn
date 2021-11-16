SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler1D t_f;
uniform highp isampler1D t_i;
uniform highp usampler1D t_u;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:4: 'sampler1D' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



