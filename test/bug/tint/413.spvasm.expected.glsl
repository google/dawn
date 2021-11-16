SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp usampler2D Src;
uniform highp writeonly image2D Dst;

void main_1() {
  uvec4 srcValue = uvec4(0u, 0u, 0u, 0u);
  uvec4 x_18 = texelFetch(Src, ivec3(0, 0, 0));
  srcValue = x_18;
  uint x_22 = srcValue.x;
  srcValue.x = (x_22 + uint(1));
  imageStore(Dst, ivec2(0, 0), srcValue).x;
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:9: 'texelFetch' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of uint'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



