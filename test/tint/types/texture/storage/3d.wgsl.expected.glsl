SKIP: FAILED

#version 310 es

layout(rgba8) uniform highp writeonly image3D t_rgba8unorm;
layout(rgba8_snorm) uniform highp writeonly image3D t_rgba8snorm;
layout(rgba8ui) uniform highp writeonly uimage3D t_rgba8uint;
layout(rgba8i) uniform highp writeonly iimage3D t_rgba8sint;
layout(rgba16ui) uniform highp writeonly uimage3D t_rgba16uint;
layout(rgba16i) uniform highp writeonly iimage3D t_rgba16sint;
layout(rgba16f) uniform highp writeonly image3D t_rgba16float;
layout(r32ui) uniform highp writeonly uimage3D t_r32uint;
layout(r32i) uniform highp writeonly iimage3D t_r32sint;
layout(r32f) uniform highp writeonly image3D t_r32float;
layout(rg32ui) uniform highp writeonly uimage3D t_rg32uint;
layout(rg32i) uniform highp writeonly iimage3D t_rg32sint;
layout(rg32f) uniform highp writeonly image3D t_rg32float;
layout(rgba32ui) uniform highp writeonly uimage3D t_rgba32uint;
layout(rgba32i) uniform highp writeonly iimage3D t_rgba32sint;
layout(rgba32f) uniform highp writeonly image3D t_rgba32float;
void tint_symbol() {
  uvec3 dim1 = uvec3(imageSize(t_rgba8unorm));
  uvec3 dim2 = uvec3(imageSize(t_rgba8snorm));
  uvec3 dim3 = uvec3(imageSize(t_rgba8uint));
  uvec3 dim4 = uvec3(imageSize(t_rgba8sint));
  uvec3 dim5 = uvec3(imageSize(t_rgba16uint));
  uvec3 dim6 = uvec3(imageSize(t_rgba16sint));
  uvec3 dim7 = uvec3(imageSize(t_rgba16float));
  uvec3 dim8 = uvec3(imageSize(t_r32uint));
  uvec3 dim9 = uvec3(imageSize(t_r32sint));
  uvec3 dim10 = uvec3(imageSize(t_r32float));
  uvec3 dim11 = uvec3(imageSize(t_rg32uint));
  uvec3 dim12 = uvec3(imageSize(t_rg32sint));
  uvec3 dim13 = uvec3(imageSize(t_rg32float));
  uvec3 dim14 = uvec3(imageSize(t_rgba32uint));
  uvec3 dim15 = uvec3(imageSize(t_rgba32sint));
  uvec3 dim16 = uvec3(imageSize(t_rgba32float));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'image load-store format' : not supported with this profile: es
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



