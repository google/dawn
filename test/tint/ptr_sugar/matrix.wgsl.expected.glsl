#version 310 es

void deref() {
  mat2x3 a = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec3 b = a[0];
  a[0] = vec3(1.0f, 2.0f, 3.0f);
}

void no_deref() {
  mat2x3 a = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec3 b = a[0];
  a[0] = vec3(1.0f, 2.0f, 3.0f);
}

void tint_symbol() {
  deref();
  no_deref();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
