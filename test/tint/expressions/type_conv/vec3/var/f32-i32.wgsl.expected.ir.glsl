#version 310 es

vec3 u = vec3(1.0f);
ivec3 tint_v3f32_to_v3i32(vec3 value) {
  ivec3 v_1 = ivec3(value);
  ivec3 v_2 = mix(ivec3((-2147483647 - 1)), v_1, greaterThanEqual(value, vec3(-2147483648.0f)));
  return mix(ivec3(2147483647), v_2, lessThanEqual(value, vec3(2147483520.0f)));
}
void f() {
  ivec3 v = tint_v3f32_to_v3i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
