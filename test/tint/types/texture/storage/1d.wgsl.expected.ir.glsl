SKIP: FAILED

#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image1D t_rgba8unorm;
layout(binding = 1, rgba8_snorm) uniform highp writeonly image1D t_rgba8snorm;
layout(binding = 2, rgba8ui) uniform highp writeonly uimage1D t_rgba8uint;
layout(binding = 3, rgba8i) uniform highp writeonly iimage1D t_rgba8sint;
layout(binding = 4, rgba16ui) uniform highp writeonly uimage1D t_rgba16uint;
layout(binding = 5, rgba16i) uniform highp writeonly iimage1D t_rgba16sint;
layout(binding = 6, rgba16f) uniform highp writeonly image1D t_rgba16float;
layout(binding = 7, r32ui) uniform highp writeonly uimage1D t_r32uint;
layout(binding = 8, r32i) uniform highp writeonly iimage1D t_r32sint;
layout(binding = 9, r32f) uniform highp writeonly image1D t_r32float;
layout(binding = 10, rg32ui) uniform highp writeonly uimage1D t_rg32uint;
layout(binding = 11, rg32i) uniform highp writeonly iimage1D t_rg32sint;
layout(binding = 12, rg32f) uniform highp writeonly image1D t_rg32float;
layout(binding = 13, rgba32ui) uniform highp writeonly uimage1D t_rgba32uint;
layout(binding = 14, rgba32i) uniform highp writeonly iimage1D t_rgba32sint;
layout(binding = 15, rgba32f) uniform highp writeonly image1D t_rgba32float;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint dim1 = uint(imageSize(t_rgba8unorm));
  uint dim2 = uint(imageSize(t_rgba8snorm));
  uint dim3 = uint(imageSize(t_rgba8uint));
  uint dim4 = uint(imageSize(t_rgba8sint));
  uint dim5 = uint(imageSize(t_rgba16uint));
  uint dim6 = uint(imageSize(t_rgba16sint));
  uint dim7 = uint(imageSize(t_rgba16float));
  uint dim8 = uint(imageSize(t_r32uint));
  uint dim9 = uint(imageSize(t_r32sint));
  uint dim10 = uint(imageSize(t_r32float));
  uint dim11 = uint(imageSize(t_rg32uint));
  uint dim12 = uint(imageSize(t_rg32sint));
  uint dim13 = uint(imageSize(t_rg32float));
  uint dim14 = uint(imageSize(t_rgba32uint));
  uint dim15 = uint(imageSize(t_rgba32sint));
  uint dim16 = uint(imageSize(t_rgba32float));
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'image1D' : Reserved word. 
WARNING: 0:3: 'layout' : useless application of layout qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
