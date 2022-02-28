vk-gl-cts/graphicsfuzz/cov-color-output-undefined-in-unexecuted-branch/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<f32, 3>;
            ^^^^^^

vk-gl-cts/graphicsfuzz/cov-color-output-undefined-in-unexecuted-branch/0-opt.wgsl:7:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_1 = @stride(16) array<i32, 4>;
              ^^^^^^

vk-gl-cts/graphicsfuzz/cov-color-output-undefined-in-unexecuted-branch/0-opt.wgsl:17:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_2 = @stride(16) array<u32, 1>;
              ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  float el;
};

struct buf1 {
  tint_padded_array_element x_GLF_uniform_float_values[3];
};

struct tint_padded_array_element_1 {
  int el;
};

struct buf2 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
};

struct buf3 {
  int three;
};

struct tint_padded_array_element_2 {
  uint el;
};

struct buf0 {
  tint_padded_array_element_2 x_GLF_uniform_uint_values[1];
};

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_float_values[3];
} x_8;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 2) uniform buf2_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
} x_12;

layout(binding = 3) uniform buf3_1 {
  int three;
} x_14;

layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element_2 x_GLF_uniform_uint_values[1];
} x_16;

void func0_() {
  vec4 tmp = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_112 = tint_symbol.x;
  float x_114 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_112 > x_114)) {
    tmp = x_GLF_color;
  }
  x_GLF_color = tmp;
  return;
}

int func1_() {
  int a = 0;
  int x_122 = x_12.x_GLF_uniform_int_values[1].el;
  a = x_122;
  while (true) {
    int x_127 = a;
    int x_129 = x_12.x_GLF_uniform_int_values[3].el;
    if ((x_127 < x_129)) {
    } else {
      break;
    }
    int x_133 = x_14.three;
    int x_135 = x_12.x_GLF_uniform_int_values[1].el;
    if ((x_133 > x_135)) {
      func0_();
      int x_142 = x_12.x_GLF_uniform_int_values[3].el;
      a = x_142;
    } else {
      func0_();
    }
  }
  return a;
}

void main_1() {
  int a_1 = 0;
  int i = 0;
  int j = 0;
  float x_56 = tint_symbol.x;
  float x_58 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_56 > x_58)) {
    float x_64 = x_8.x_GLF_uniform_float_values[0].el;
    float x_66 = x_8.x_GLF_uniform_float_values[1].el;
    float x_68 = x_8.x_GLF_uniform_float_values[0].el;
    float x_70 = x_8.x_GLF_uniform_float_values[2].el;
    x_GLF_color = vec4(x_64, x_66, x_68, x_70);
  } else {
    uint x_73 = x_16.x_GLF_uniform_uint_values[0].el;
    x_GLF_color = unpackSnorm4x8(x_73);
  }
  int x_76 = x_12.x_GLF_uniform_int_values[2].el;
  a_1 = x_76;
  i = 0;
  {
    for(; (i < 5); i = (i + 1)) {
      j = 0;
      {
        for(; (j < 2); j = (j + 1)) {
          int x_91 = func1_();
          a_1 = (a_1 + x_91);
        }
      }
    }
  }
  int x_98 = a_1;
  int x_100 = x_12.x_GLF_uniform_int_values[0].el;
  if ((x_98 == x_100)) {
    float x_105 = x_8.x_GLF_uniform_float_values[0].el;
    float x_107 = x_GLF_color.z;
    x_GLF_color.z = (x_107 - x_105);
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
