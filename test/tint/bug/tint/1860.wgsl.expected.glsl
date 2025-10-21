#version 310 es

layout(binding = 0, std140)
uniform v_declared_after_usage_block_ubo {
  uvec4 inner[1];
} v;
vec4 main_inner() {
  uvec4 v_1 = v.inner[0u];
  return vec4(uintBitsToFloat(v_1.x));
}
void main() {
  vec4 v_2 = main_inner();
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
