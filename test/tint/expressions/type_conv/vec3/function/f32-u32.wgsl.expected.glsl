#version 310 es

float t = 0.0f;
vec3 m() {
  t = 1.0f;
  return vec3(t);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  return uvec3(clamp(value, vec3(0.0f), vec3(4294967040.0f)));
}
void f() {
  uvec3 v = tint_v3f32_to_v3u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
