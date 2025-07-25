#version 310 es

float t = 0.0f;
vec4 m() {
  t = 1.0f;
  return vec4(t);
}
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  return uvec4(clamp(value, vec4(0.0f), vec4(4294967040.0f)));
}
void f() {
  uvec4 v = tint_v4f32_to_v4u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
