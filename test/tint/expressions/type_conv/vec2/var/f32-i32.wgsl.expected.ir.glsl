#version 310 es

vec2 u = vec2(1.0f);
ivec2 tint_v2f32_to_v2i32(vec2 value) {
  ivec2 v_1 = ivec2(value);
  int v_2 = (((value >= vec2(-2147483648.0f)).x) ? (v_1.x) : (ivec2((-2147483647 - 1)).x));
  ivec2 v_3 = ivec2(v_2, (((value >= vec2(-2147483648.0f)).y) ? (v_1.y) : (ivec2((-2147483647 - 1)).y)));
  int v_4 = (((value <= vec2(2147483520.0f)).x) ? (v_3.x) : (ivec2(2147483647).x));
  return ivec2(v_4, (((value <= vec2(2147483520.0f)).y) ? (v_3.y) : (ivec2(2147483647).y)));
}
void f() {
  ivec2 v = tint_v2f32_to_v2i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
