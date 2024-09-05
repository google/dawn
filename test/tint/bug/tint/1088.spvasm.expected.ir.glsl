#version 310 es


struct strided_arr {
  float el;
};

struct LeftOver {
  mat4 worldViewProjection;
  float time;
  mat4 test2[2];
  strided_arr test[4];
};

struct main_out {
  vec4 tint_symbol;
  vec2 vUV_1;
};

vec3 position_1 = vec3(0.0f);
layout(binding = 2, std140)
uniform tint_symbol_3_1_ubo {
  LeftOver tint_symbol_2;
} v;
vec2 vUV = vec2(0.0f);
vec2 uv = vec2(0.0f);
vec3 normal = vec3(0.0f);
vec4 tint_symbol = vec4(0.0f);
layout(location = 0) in vec3 tint_symbol_1_loc0_Input;
layout(location = 2) in vec2 tint_symbol_1_loc2_Input;
layout(location = 1) in vec3 tint_symbol_1_loc1_Input;
layout(location = 0) out vec2 tint_symbol_1_loc0_Output;
void main_1() {
  vec4 q = vec4(0.0f);
  vec3 p = vec3(0.0f);
  q = vec4(position_1.x, position_1.y, position_1.z, 1.0f);
  p = q.xyz;
  float v_1 = p.x;
  p[0u] = (v_1 + sin(((v.tint_symbol_2.test[0].el * position_1.y) + v.tint_symbol_2.time)));
  float v_2 = p.y;
  p[1u] = (v_2 + sin((v.tint_symbol_2.time + 4.0f)));
  mat4 v_3 = v.tint_symbol_2.worldViewProjection;
  tint_symbol = (v_3 * vec4(p.x, p.y, p.z, 1.0f));
  vUV = uv;
  tint_symbol[1u] = (tint_symbol.y * -1.0f);
}
main_out tint_symbol_1_inner(vec3 position_1_param, vec2 uv_param, vec3 normal_param) {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  return main_out(tint_symbol, vUV);
}
void main() {
  main_out v_4 = tint_symbol_1_inner(tint_symbol_1_loc0_Input, tint_symbol_1_loc2_Input, tint_symbol_1_loc1_Input);
  gl_Position = v_4.tint_symbol;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_symbol_1_loc0_Output = v_4.vUV_1;
  gl_PointSize = 1.0f;
}
