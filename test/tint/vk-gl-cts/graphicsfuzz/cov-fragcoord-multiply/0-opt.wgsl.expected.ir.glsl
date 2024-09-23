SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};

vec4 tint_symbol = vec4(0.0f);
layout(binding = 1, std140)
uniform tint_symbol_4_1_ubo {
  buf1 tint_symbol_3;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_6_1_ubo {
  buf0 tint_symbol_5;
} v_1;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
ivec2 tint_v2f32_to_v2i32(vec2 value) {
  ivec2 v_2 = ivec2(value);
  int v_3 = (((value >= vec2(-2147483648.0f)).x) ? (v_2.x) : (ivec2((-2147483647 - 1)).x));
  ivec2 v_4 = ivec2(v_3, (((value >= vec2(-2147483648.0f)).y) ? (v_2.y) : (ivec2((-2147483647 - 1)).y)));
  int v_5 = (((value <= vec2(2147483520.0f)).x) ? (v_4.x) : (ivec2(2147483647).x));
  return ivec2(v_5, (((value <= vec2(2147483520.0f)).y) ? (v_4.y) : (ivec2(2147483647).y)));
}
void main_1() {
  ivec2 icoord = ivec2(0);
  float x_40 = 0.0f;
  ivec2 icoord_1 = ivec2(0);
  float x_42 = tint_symbol.x;
  float x_44 = v.tint_symbol_3.x_GLF_uniform_float_values[1].el;
  float x_47 = v.tint_symbol_3.x_GLF_uniform_float_values[0].el;
  if (((x_42 * x_44) > x_47)) {
    vec4 x_52 = tint_symbol;
    float x_55 = v.tint_symbol_3.x_GLF_uniform_float_values[1].el;
    float x_58 = v.tint_symbol_3.x_GLF_uniform_float_values[0].el;
    float x_60 = v.tint_symbol_3.x_GLF_uniform_float_values[2].el;
    vec2 v_6 = (vec2(x_52[0u], x_52[1u]) * x_55);
    icoord = tint_v2f32_to_v2i32((v_6 - vec2(x_58, x_60)));
    float x_65 = v.tint_symbol_3.x_GLF_uniform_float_values[2].el;
    float x_67 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    int x_69 = icoord.x;
    int x_71 = icoord.y;
    int x_74 = v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el;
    if (((x_69 * x_71) != x_74)) {
      float x_80 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
      x_40 = x_80;
    } else {
      float x_82 = v.tint_symbol_3.x_GLF_uniform_float_values[2].el;
      x_40 = x_82;
    }
    float x_83 = x_40;
    int x_85 = v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(x_65, x_67, x_83, float(x_85));
  } else {
    vec4 x_88 = tint_symbol;
    float x_91 = v.tint_symbol_3.x_GLF_uniform_float_values[1].el;
    float x_94 = v.tint_symbol_3.x_GLF_uniform_float_values[0].el;
    float x_96 = v.tint_symbol_3.x_GLF_uniform_float_values[2].el;
    vec2 v_7 = (vec2(x_88[0u], x_88[1u]) * x_91);
    icoord_1 = tint_v2f32_to_v2i32((v_7 - vec2(x_94, x_96)));
    float x_101 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    float x_103 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    int x_105 = icoord_1.x;
    float x_108 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    x_GLF_color = vec4(x_101, x_103, float(x_105), x_108);
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
