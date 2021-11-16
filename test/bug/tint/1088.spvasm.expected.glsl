#version 310 es
precision mediump float;

struct tint_padded_array_element {
  float el;
};

vec3 position = vec3(0.0f, 0.0f, 0.0f);
layout (binding = 2) uniform LeftOver_1 {
  mat4 worldViewProjection;
  float time;
  mat4 test2[2];
  tint_padded_array_element test[4];
} x_14;
vec2 vUV = vec2(0.0f, 0.0f);
vec2 uv = vec2(0.0f, 0.0f);
vec3 normal = vec3(0.0f, 0.0f, 0.0f);
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  vec4 q = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec3 p = vec3(0.0f, 0.0f, 0.0f);
  vec3 x_13 = position;
  q = vec4(x_13.x, x_13.y, x_13.z, 1.0f);
  vec4 x_21 = q;
  p = vec3(x_21.x, x_21.y, x_21.z);
  float x_27 = p.x;
  float x_41 = x_14.test[0].el;
  float x_45 = position.y;
  float x_49 = x_14.time;
  p.x = (x_27 + sin(((x_41 * x_45) + x_49)));
  float x_55 = p.y;
  float x_57 = x_14.time;
  p.y = (x_55 + sin((x_57 + 4.0f)));
  mat4 x_69 = x_14.worldViewProjection;
  vec3 x_70 = p;
  tint_symbol = (x_69 * vec4(x_70.x, x_70.y, x_70.z, 1.0f));
  vUV = uv;
  float x_87 = tint_symbol.y;
  tint_symbol.y = (x_87 * -1.0f);
  return;
}

struct main_out {
  vec4 tint_symbol;
  vec2 vUV_1;
};
struct tint_symbol_3 {
  vec3 position_param;
  vec3 normal_param;
  vec2 uv_param;
};
struct tint_symbol_4 {
  vec2 vUV_1;
  vec4 tint_symbol;
};

main_out tint_symbol_1_inner(vec3 position_param, vec2 uv_param, vec3 normal_param) {
  position = position_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  main_out tint_symbol_5 = main_out(tint_symbol, vUV);
  return tint_symbol_5;
}

tint_symbol_4 tint_symbol_1(tint_symbol_3 tint_symbol_2) {
  main_out inner_result = tint_symbol_1_inner(tint_symbol_2.position_param, tint_symbol_2.uv_param, tint_symbol_2.normal_param);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.tint_symbol = inner_result.tint_symbol;
  wrapper_result.vUV_1 = inner_result.vUV_1;
  return wrapper_result;
}
in vec3 position_param;
in vec3 normal_param;
in vec2 uv_param;
out vec2 vUV_1;
void main() {
  tint_symbol_3 inputs;
  inputs.position_param = position_param;
  inputs.normal_param = normal_param;
  inputs.uv_param = uv_param;
  tint_symbol_4 outputs;
  outputs = tint_symbol_1(inputs);
  vUV_1 = outputs.vUV_1;
  gl_Position = outputs.tint_symbol;
  gl_Position.y = -gl_Position.y;
}


