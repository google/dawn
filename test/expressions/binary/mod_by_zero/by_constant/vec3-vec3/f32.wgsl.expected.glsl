SKIP: FAILED

#version 310 es

void f() {
  vec3 a = vec3(1.0f, 2.0f, 3.0f);
  vec3 b = vec3(0.0f, 5.0f, 0.0f);
  vec3 r = (a % b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' temp highp 3-component vector of float' and a right operand of type ' temp highp 3-component vector of float' (or there is no acceptable conversion)
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



