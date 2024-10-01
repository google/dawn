SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
layout(binding = 0, r32i) uniform highp iimage1D arg_0;
ivec4 textureLoad_c80691() {
  uint arg_1 = 1u;
  ivec4 res = imageLoad(arg_0, int(arg_1));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_c80691();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'iimage1D' : Reserved word. 
WARNING: 0:9: 'layout' : useless application of layout qualifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
layout(binding = 0, r32i) uniform highp iimage1D arg_0;
ivec4 textureLoad_c80691() {
  uint arg_1 = 1u;
  ivec4 res = imageLoad(arg_0, int(arg_1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_c80691();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'iimage1D' : Reserved word. 
WARNING: 0:7: 'layout' : useless application of layout qualifier 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
