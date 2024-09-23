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
  float x_39 = tint_symbol.y;
  float x_41 = v_1.tint_symbol_3.x_GLF_uniform_float_values[0].el;
  if ((x_39 < x_41)) {
    int x_47 = v_2.tint_symbol_5.x_GLF_uniform_int_values[0].el;
    float x_48 = float(x_47);
    x_GLF_color = vec4(x_48, x_48, x_48, x_48);
  } else {
    vec4 x_50 = tint_symbol;
    float x_53 = v_1.tint_symbol_3.x_GLF_uniform_float_values[1].el;
    float x_55 = v_1.tint_symbol_3.x_GLF_uniform_float_values[0].el;
    float x_59 = v_1.tint_symbol_3.x_GLF_uniform_float_values[2].el;
    vec2 v_7 = vec2(x_50[0u], x_50[1u]);
    v = tint_v2f32_to_v2i32(((v_7 - vec2(x_53, x_55)) * x_59));
    float x_63 = v_1.tint_symbol_3.x_GLF_uniform_float_values[1].el;
    int x_65 = v.y;
    int x_67 = v_2.tint_symbol_5.x_GLF_uniform_int_values[1].el;
    int x_70 = v_2.tint_symbol_5.x_GLF_uniform_int_values[1].el;
    int x_74 = v.x;
    int x_76 = v_2.tint_symbol_5.x_GLF_uniform_int_values[1].el;
    float x_80 = v_1.tint_symbol_3.x_GLF_uniform_float_values[1].el;
    float v_8 = float(((x_65 - x_67) & x_70));
    x_GLF_color = vec4(x_63, v_8, float((x_74 & x_76)), x_80);
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
