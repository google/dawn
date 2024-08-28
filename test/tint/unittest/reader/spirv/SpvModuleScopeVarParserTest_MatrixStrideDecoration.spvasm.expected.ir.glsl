SKIP: FAILED

#version 310 es

struct strided_arr {
  vec2 el;
};

struct S {
  strided_arr field0[3];
};

S myvar = S(strided_arr[3](strided_arr(vec2(0.0f)), strided_arr(vec2(0.0f)), strided_arr(vec2(0.0f))));
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
