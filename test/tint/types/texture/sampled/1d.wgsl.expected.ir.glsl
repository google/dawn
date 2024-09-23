SKIP: FAILED

#version 310 es

uniform highp sampler1D t_f;
uniform highp isampler1D t_i;
uniform highp usampler1D t_u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint fdims = uint(textureSize(t_f, 1));
  uint idims = uint(textureSize(t_i, 1));
  uint udims = uint(textureSize(t_u, 1));
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'sampler1D' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
