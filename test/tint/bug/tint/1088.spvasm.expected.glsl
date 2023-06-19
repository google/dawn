#version 310 es

layout(location = 0) in vec3 position_1_param_1;
layout(location = 2) in vec2 uv_param_1;
layout(location = 1) in vec3 normal_param_1;
layout(location = 0) out vec2 vUV_1_1;
struct strided_arr {
  float el;
  uint pad;
  uint pad_1;
  uint pad_2;
};

struct LeftOver {
  mat4 worldViewProjection;
  float time;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  mat4 test2[2];
  strided_arr test[4];
};

vec3 position_1 = vec3(0.0f, 0.0f, 0.0f);
layout(binding = 2, std140) uniform x_14_block_ubo {
  LeftOver inner;
} x_14;

vec2 vUV = vec2(0.0f, 0.0f);
vec2 uv = vec2(0.0f, 0.0f);
vec3 normal = vec3(0.0f, 0.0f, 0.0f);
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  vec4 q = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec3 p = vec3(0.0f, 0.0f, 0.0f);
  vec3 x_13 = position_1;
  q = vec4(x_13.x, x_13.y, x_13.z, 1.0f);
  vec4 x_21 = q;
  p = x_21.xyz;
  float x_27 = p.x;
  float x_41 = x_14.inner.test[0].el;
  float x_45 = position_1.y;
  float x_49 = x_14.inner.time;
  p.x = (x_27 + sin(((x_41 * x_45) + x_49)));
  float x_55 = p.y;
  float x_57 = x_14.inner.time;
  p.y = (x_55 + sin((x_57 + 4.0f)));
  mat4 x_69 = x_14.inner.worldViewProjection;
  vec3 x_70 = p;
  tint_symbol = (x_69 * vec4(x_70.x, x_70.y, x_70.z, 1.0f));
  vec2 x_83 = uv;
  vUV = x_83;
  float x_87 = tint_symbol.y;
  tint_symbol.y = (x_87 * -1.0f);
  return;
}

struct main_out {
  vec4 tint_symbol;
  vec2 vUV_1;
};

main_out tint_symbol_1(vec3 position_1_param, vec2 uv_param, vec3 normal_param) {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  main_out tint_symbol_2 = main_out(tint_symbol, vUV);
  return tint_symbol_2;
}

void main() {
  gl_PointSize = 1.0;
  main_out inner_result = tint_symbol_1(position_1_param_1, uv_param_1, normal_param_1);
  gl_Position = inner_result.tint_symbol;
  vUV_1_1 = inner_result.vUV_1;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
