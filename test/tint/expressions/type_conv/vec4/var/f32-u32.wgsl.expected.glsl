#version 310 es

vec4 u = vec4(1.0f);
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  uvec4 v_1 = uvec4(value);
  uvec4 v_2 = mix(uvec4(0u), v_1, greaterThanEqual(value, vec4(0.0f)));
  return mix(uvec4(4294967295u), v_2, lessThanEqual(value, vec4(4294967040.0f)));
}
void f() {
  uvec4 v = tint_v4f32_to_v4u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
