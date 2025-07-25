#version 310 es

vec2 u = vec2(1.0f);
ivec2 tint_v2f32_to_v2i32(vec2 value) {
  return ivec2(clamp(value, vec2(-2147483648.0f), vec2(2147483520.0f)));
}
void f() {
  ivec2 v = tint_v2f32_to_v2i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
