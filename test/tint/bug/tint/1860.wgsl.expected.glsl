#version 310 es

struct DeclaredAfterUsage {
  float f;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std140) uniform declared_after_usage_block_ubo {
  DeclaredAfterUsage inner;
} declared_after_usage;

vec4 tint_symbol() {
  return vec4(declared_after_usage.inner.f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
