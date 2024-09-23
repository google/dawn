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
  if (((tint_symbol.x * v.tint_symbol_3.x_GLF_uniform_float_values[1].el) > v.tint_symbol_3.x_GLF_uniform_float_values[0].el)) {
    vec2 v_6 = (tint_symbol.xy * v.tint_symbol_3.x_GLF_uniform_float_values[1].el);
    icoord = tint_v2f32_to_v2i32((v_6 - vec2(v.tint_symbol_3.x_GLF_uniform_float_values[0].el, v.tint_symbol_3.x_GLF_uniform_float_values[2].el)));
    float x_65 = v.tint_symbol_3.x_GLF_uniform_float_values[2].el;
    float x_67 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    if (((icoord.x * icoord.y) != v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el)) {
      x_40 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    } else {
      x_40 = v.tint_symbol_3.x_GLF_uniform_float_values[2].el;
    }
    float v_7 = x_40;
    x_GLF_color = vec4(x_65, x_67, v_7, float(v_1.tint_symbol_5.x_GLF_uniform_int_values[0].el));
  } else {
    vec2 v_8 = (tint_symbol.xy * v.tint_symbol_3.x_GLF_uniform_float_values[1].el);
    icoord_1 = tint_v2f32_to_v2i32((v_8 - vec2(v.tint_symbol_3.x_GLF_uniform_float_values[0].el, v.tint_symbol_3.x_GLF_uniform_float_values[2].el)));
    float v_9 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    float v_10 = v.tint_symbol_3.x_GLF_uniform_float_values[3].el;
    float v_11 = float(icoord_1.x);
    x_GLF_color = vec4(v_9, v_10, v_11, v.tint_symbol_3.x_GLF_uniform_float_values[3].el);
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
