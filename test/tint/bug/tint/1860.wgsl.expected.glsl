#version 310 es


struct DeclaredAfterUsage {
  float f;
};

layout(binding = 0, std140)
uniform v_declared_after_usage_block_ubo {
  DeclaredAfterUsage inner;
} v;
vec4 main_inner() {
  return vec4(v.inner.f);
}
void main() {
  gl_Position = main_inner();
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
