#version 310 es

void tint_symbol() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  float scalar = vec3(1.0f, 2.0f, 3.0f).y;
  vec2 swizzle2 = vec3(1.0f, 2.0f, 3.0f).xz;
  vec3 swizzle3 = vec3(1.0f, 2.0f, 3.0f).xzy;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
