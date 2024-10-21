#version 310 es

vec3 u = vec3(1.0f);
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  uvec3 v_1 = uvec3(value);
  uvec3 v_2 = mix(uvec3(0u), v_1, greaterThanEqual(value, vec3(0.0f)));
  return mix(uvec3(4294967295u), v_2, lessThanEqual(value, vec3(4294967040.0f)));
}
void f() {
  uvec3 v = tint_v3f32_to_v3u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
