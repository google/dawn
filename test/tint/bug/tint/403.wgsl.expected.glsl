#version 310 es

struct vertexUniformBuffer1 {
  mat2 transform1;
};

struct vertexUniformBuffer1_std140 {
  vec2 transform1_0;
  vec2 transform1_1;
};

struct vertexUniformBuffer2 {
  mat2 transform2;
};

struct vertexUniformBuffer2_std140 {
  vec2 transform2_0;
  vec2 transform2_1;
};

layout(binding = 0, std140) uniform x_20_block_std140_ubo {
  vertexUniformBuffer1_std140 inner;
} x_20;

layout(binding = 0, std140) uniform x_26_block_std140_ubo {
  vertexUniformBuffer2_std140 inner;
} x_26;

mat2 load_x_20_inner_transform1() {
  return mat2(x_20.inner.transform1_0, x_20.inner.transform1_1);
}

mat2 load_x_26_inner_transform2() {
  return mat2(x_26.inner.transform2_0, x_26.inner.transform2_1);
}

vec4 tint_symbol(uint tint_symbol_1) {
  vec2 indexable[3] = vec2[3](vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f));
  mat2 x_23 = load_x_20_inner_transform1();
  mat2 x_28 = load_x_26_inner_transform2();
  uint x_46 = tint_symbol_1;
  vec2 tint_symbol_2[3] = vec2[3](vec2(-1.0f, 1.0f), vec2(1.0f), vec2(-1.0f));
  indexable = tint_symbol_2;
  vec2 x_51 = indexable[x_46];
  vec2 x_52 = (mat2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])) * x_51);
  return vec4(x_52.x, x_52.y, 0.0f, 1.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol(uint(gl_VertexID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
