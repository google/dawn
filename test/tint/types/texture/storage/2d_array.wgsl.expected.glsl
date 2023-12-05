SKIP: FAILED

#version 310 es

layout(rgba8) uniform highp writeonly image2DArray t_rgba8unorm;
layout(rgba8_snorm) uniform highp writeonly image2DArray t_rgba8snorm;
layout(rgba8ui) uniform highp writeonly uimage2DArray t_rgba8uint;
layout(rgba8i) uniform highp writeonly iimage2DArray t_rgba8sint;
layout(rgba16ui) uniform highp writeonly uimage2DArray t_rgba16uint;
layout(rgba16i) uniform highp writeonly iimage2DArray t_rgba16sint;
layout(rgba16f) uniform highp writeonly image2DArray t_rgba16float;
layout(r32ui) uniform highp writeonly uimage2DArray t_r32uint;
layout(r32i) uniform highp writeonly iimage2DArray t_r32sint;
layout(r32f) uniform highp writeonly image2DArray t_r32float;
layout(rg32ui) uniform highp writeonly uimage2DArray t_rg32uint;
layout(rg32i) uniform highp writeonly iimage2DArray t_rg32sint;
layout(rg32f) uniform highp writeonly image2DArray t_rg32float;
layout(rgba32ui) uniform highp writeonly uimage2DArray t_rgba32uint;
layout(rgba32i) uniform highp writeonly iimage2DArray t_rgba32sint;
layout(rgba32f) uniform highp writeonly image2DArray t_rgba32float;
void tint_symbol() {
  uvec2 dim1 = uvec2(imageSize(t_rgba8unorm).xy);
  uvec2 dim2 = uvec2(imageSize(t_rgba8snorm).xy);
  uvec2 dim3 = uvec2(imageSize(t_rgba8uint).xy);
  uvec2 dim4 = uvec2(imageSize(t_rgba8sint).xy);
  uvec2 dim5 = uvec2(imageSize(t_rgba16uint).xy);
  uvec2 dim6 = uvec2(imageSize(t_rgba16sint).xy);
  uvec2 dim7 = uvec2(imageSize(t_rgba16float).xy);
  uvec2 dim8 = uvec2(imageSize(t_r32uint).xy);
  uvec2 dim9 = uvec2(imageSize(t_r32sint).xy);
  uvec2 dim10 = uvec2(imageSize(t_r32float).xy);
  uvec2 dim11 = uvec2(imageSize(t_rg32uint).xy);
  uvec2 dim12 = uvec2(imageSize(t_rg32sint).xy);
  uvec2 dim13 = uvec2(imageSize(t_rg32float).xy);
  uvec2 dim14 = uvec2(imageSize(t_rgba32uint).xy);
  uvec2 dim15 = uvec2(imageSize(t_rgba32sint).xy);
  uvec2 dim16 = uvec2(imageSize(t_rgba32float).xy);
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



