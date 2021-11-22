SKIP: FAILED

#version 310 es
precision mediump float;

bvec4 tint_isNormal(vec4 param_0) {
  uint4 exponent = asuint(param_0) & 0x7f80000;
  uint4 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

bvec3 tint_isNormal_1(vec3 param_0) {
  uint3 exponent = asuint(param_0) & 0x7f80000;
  uint3 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

bvec2 tint_isNormal_2(vec2 param_0) {
  uint2 exponent = asuint(param_0) & 0x7f80000;
  uint2 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

bool tint_isNormal_3(float param_0) {
  uint exponent = asuint(param_0) & 0x7f80000;
  uint clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  tint_isNormal(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_isNormal(vec4(1.0f));
  tint_isNormal(vec4(1.0f, 2.0f, 3.0f, 4.0f));
  tint_isNormal_1(vec3(0.0f, 0.0f, 0.0f));
  tint_isNormal_1(vec3(1.0f));
  tint_isNormal_1(vec3(1.0f, 2.0f, 3.0f));
  tint_isNormal_2(vec2(0.0f, 0.0f));
  tint_isNormal_2(vec2(1.0f));
  tint_isNormal_2(vec2(1.0f, 2.0f));
  tint_isNormal_3(1.0f);
  tint_isNormal_3(2.0f);
  tint_isNormal_3(3.0f);
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:5: 'uint4' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



