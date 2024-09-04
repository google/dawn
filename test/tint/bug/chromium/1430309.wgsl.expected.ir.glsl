SKIP: FAILED

#version 310 es

struct frexp_result_f32 {
  float f;
};

struct frexp_result_f32_1 {
  float fract;
  int exp;
};
precision highp float;
precision highp int;


frexp_result_f32 a = frexp_result_f32(0.0f);
frexp_result_f32_1 b = frexp_result_f32_1(0.5f, 1);
vec4 main() {
  return vec4(a.f, b.fract, 0.0f, 0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
