SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
layout(binding = 0, rgba32f) uniform highp writeonly image1D arg_0;
uint textureDimensions_da30d2() {
  uint res = uint(imageSize(arg_0));
  return res;
}
void main() {
  v.tint_symbol = textureDimensions_da30d2();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'image1D' : Reserved word. 
WARNING: 0:9: 'layout' : useless application of layout qualifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
layout(binding = 0, rgba32f) uniform highp writeonly image1D arg_0;
uint textureDimensions_da30d2() {
  uint res = uint(imageSize(arg_0));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureDimensions_da30d2();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'image1D' : Reserved word. 
WARNING: 0:7: 'layout' : useless application of layout qualifier 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
