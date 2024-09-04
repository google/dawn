SKIP: FAILED

#version 310 es

struct S {
  vec4 field0;
};

struct S_1 {
  vec4 field0;
};

struct main_out {
  vec4 x_3_1;
};
precision highp float;
precision highp int;


vec4 x_3 = vec4(0.0f);
uniform S x_5;
void main_1() {
  vec4 x_20 = x_5.field0;
  S_1 x_21_1 = S_1(vec4(0.0f));
  x_21_1.field0 = x_20;
  x_3 = x_21_1.field0;
}
main_out main() {
  main_1();
  return main_out(x_3);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
