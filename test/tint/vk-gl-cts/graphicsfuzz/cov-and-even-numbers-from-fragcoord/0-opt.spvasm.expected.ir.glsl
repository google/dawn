SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};

vec4 tint_symbol = vec4(0.0f);
layout(binding = 1, std140)
uniform tint_symbol_4_1_ubo {
  buf1 tint_symbol_3;
} v_1;
vec4 x_GLF_color = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_6_1_ubo {
  buf0 tint_symbol_5;
} v_2;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
ivec2 tint_v2f32_to_v2i32(vec2 value) {
  ivec2 v_3 = ivec2(value);
  int v_4 = (((value >= vec2(-2147483648.0f)).x) ? (v_3.x) : (ivec2((-2147483647 - 1)).x));
  ivec2 v_5 = ivec2(v_4, (((value >= vec2(-2147483648.0f)).y) ? (v_3.y) : (ivec2((-2147483647 - 1)).y)));
  int v_6 = (((value <= vec2(2147483520.0f)).x) ? (v_5.x) : (ivec2(2147483647).x));
  return ivec2(v_6, (((value <= vec2(2147483520.0f)).y) ? (v_5.y) : (ivec2(2147483647).y)));
}
void main_1() {
  ivec2 v = ivec2(0);
  if ((tint_symbol.y < v_1.tint_symbol_3.x_GLF_uniform_float_values[0].el)) {
    x_GLF_color = vec4(float(v_2.tint_symbol_5.x_GLF_uniform_int_values[0].el));
  } else {
    vec2 v_7 = tint_symbol.xy;
    vec2 v_8 = (v_7 - vec2(v_1.tint_symbol_3.x_GLF_uniform_float_values[1].el, v_1.tint_symbol_3.x_GLF_uniform_float_values[0].el));
    v = tint_v2f32_to_v2i32((v_8 * v_1.tint_symbol_3.x_GLF_uniform_float_values[2].el));
    float v_9 = v_1.tint_symbol_3.x_GLF_uniform_float_values[1].el;
    float v_10 = float(((v.y - v_2.tint_symbol_5.x_GLF_uniform_int_values[1].el) & v_2.tint_symbol_5.x_GLF_uniform_int_values[1].el));
    float v_11 = float((v.x & v_2.tint_symbol_5.x_GLF_uniform_int_values[1].el));
    x_GLF_color = vec4(v_9, v_10, v_11, v_1.tint_symbol_3.x_GLF_uniform_float_values[1].el);
  }
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:39: '>=' :  wrong operand types: no operation '>=' exists that takes a left-hand operand of type ' in highp 2-component vector of float' and a right operand of type ' const 2-component vector of float' (or there is no acceptable conversion)
ERROR: 0:39: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
