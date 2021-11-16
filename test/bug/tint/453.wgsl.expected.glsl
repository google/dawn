SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp usampler2D Src;
uniform highp writeonly image2D Dst;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  uvec4 srcValue = uvec4(0u, 0u, 0u, 0u);
  uvec4 x_22 = texelFetch(Src, ivec3(0, 0, 0));
  srcValue = x_22;
  uint x_24 = srcValue.x;
  uint x_25 = (x_24 + 1u);
  imageStore(Dst, ivec2(0, 0), srcValue.xxxx).x;
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:10: 'texelFetch' : no matching overloaded function found 
ERROR: 0:10: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of uint'
ERROR: 0:10: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



