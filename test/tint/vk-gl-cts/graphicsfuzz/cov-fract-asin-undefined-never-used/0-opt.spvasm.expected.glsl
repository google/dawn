#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  strided_arr x_GLF_uniform_float_values[1];
} x_8;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 1) uniform buf1_1 {
  strided_arr_1 x_GLF_uniform_int_values[2];
} x_10;

void main_1() {
  float f0 = 0.0f;
  float f1 = 0.0f;
  f0 = uintBitsToFloat(0x7fc00000u);
  f1 = fract(f0);
  float x_38 = tint_symbol.x;
  float x_40 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_38 > x_40)) {
    int x_46 = x_10.x_GLF_uniform_int_values[1].el;
    int x_49 = x_10.x_GLF_uniform_int_values[0].el;
    int x_52 = x_10.x_GLF_uniform_int_values[0].el;
    int x_55 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_46), float(x_49), float(x_52), float(x_55));
  } else {
    float x_58 = f1;
    x_GLF_color = vec4(x_58, x_58, x_58, x_58);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_1(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_3 = main_out(x_GLF_color);
  return tint_symbol_3;
}

void main() {
  main_out inner_result = tint_symbol_1(gl_FragCoord);
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
