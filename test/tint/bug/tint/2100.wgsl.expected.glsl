#version 310 es

layout(binding = 0, std140)
uniform v_buffer_block_ubo {
  uvec4 inner[7];
} v;
vec4 main_inner() {
  uvec4 v_1 = v.inner[0u];
  float x = uintBitsToFloat(v_1.z);
  return vec4(x, 0.0f, 0.0f, 1.0f);
}
void main() {
  vec4 v_2 = main_inner();
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
