#version 310 es

float tint_float_modulo(float lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


void f() {
  float a = 1.0f;
  float b = 0.0f;
  float r = tint_float_modulo(a, (b + b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
