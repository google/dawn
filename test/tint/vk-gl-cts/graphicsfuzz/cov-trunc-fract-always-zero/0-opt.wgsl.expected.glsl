vk-gl-cts/graphicsfuzz/cov-trunc-fract-always-zero/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<f32, 2>;
            ^^^^^^

vk-gl-cts/graphicsfuzz/cov-trunc-fract-always-zero/0-opt.wgsl:7:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_1 = @stride(16) array<i32, 2>;
              ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  float el;
};

struct buf1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
};

struct tint_padded_array_element_1 {
  int el;
};

struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[2];
};

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
} x_7;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[2];
} x_9;

void main_1() {
  float f = 0.0f;
  float x_35 = tint_symbol.y;
  float x_37 = x_7.x_GLF_uniform_float_values[1].el;
  f = fract(trunc(((x_35 < x_37) ? 0.100000001f : 1.0f)));
  float x_42 = f;
  float x_44 = x_7.x_GLF_uniform_float_values[0].el;
  if ((x_42 == x_44)) {
    int x_50 = x_9.x_GLF_uniform_int_values[0].el;
    int x_53 = x_9.x_GLF_uniform_int_values[1].el;
    int x_56 = x_9.x_GLF_uniform_int_values[1].el;
    int x_59 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_50), float(x_53), float(x_56), float(x_59));
  } else {
    int x_63 = x_9.x_GLF_uniform_int_values[1].el;
    float x_64 = float(x_63);
    x_GLF_color = vec4(x_64, x_64, x_64, x_64);
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
