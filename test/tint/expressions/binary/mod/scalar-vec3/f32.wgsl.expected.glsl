#version 310 es

vec3 tint_float_modulo(float lhs, vec3 rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


void f() {
  float a = 4.0f;
  vec3 b = vec3(1.0f, 2.0f, 3.0f);
  vec3 r = tint_float_modulo(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
