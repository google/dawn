vk-gl-cts/graphicsfuzz/cov-bitcount/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<f32, 1>;
            ^^^^^^

vk-gl-cts/graphicsfuzz/cov-bitcount/0-opt.wgsl:7:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_1 = @stride(16) array<i32, 3>;
              ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  float el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[1];
};

struct tint_padded_array_element_1 {
  int el;
};

struct buf1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[3];
};

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[1];
} x_8;

layout(binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[3];
} x_11;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
int f1_() {
  int a = 0;
  int i = 0;
  a = 256;
  float x_65 = tint_symbol.y;
  float x_67 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_65 > x_67)) {
    a = (a + 1);
  }
  i = int(bitCount(a));
  int x_75 = i;
  int x_77 = x_11.x_GLF_uniform_int_values[0].el;
  if ((x_75 < x_77)) {
    int x_82 = x_11.x_GLF_uniform_int_values[0].el;
    return x_82;
  }
  return i;
}

void main_1() {
  int a_1 = 0;
  int x_38 = f1_();
  a_1 = x_38;
  int x_39 = a_1;
  int x_41 = x_11.x_GLF_uniform_int_values[2].el;
  if ((x_39 == x_41)) {
    int x_47 = x_11.x_GLF_uniform_int_values[0].el;
    int x_50 = x_11.x_GLF_uniform_int_values[1].el;
    int x_53 = x_11.x_GLF_uniform_int_values[1].el;
    int x_56 = x_11.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_47), float(x_50), float(x_53), float(x_56));
  } else {
    int x_60 = x_11.x_GLF_uniform_int_values[1].el;
    float x_61 = float(x_60);
    x_GLF_color = vec4(x_61, x_61, x_61, x_61);
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
