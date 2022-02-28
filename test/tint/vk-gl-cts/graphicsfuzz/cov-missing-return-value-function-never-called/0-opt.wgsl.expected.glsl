vk-gl-cts/graphicsfuzz/cov-missing-return-value-function-never-called/0-opt.wgsl:5:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<i32, 1>;
            ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct buf1 {
  uint one;
};

struct tint_padded_array_element {
  int el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[1];
};

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 1) uniform buf1_1 {
  uint one;
} x_8;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[1];
} x_10;

float func_() {
  switch(1) {
    case 0: {
      return 1.0f;
      break;
    }
    default: {
      break;
    }
  }
  return 0.0f;
}

void main_1() {
  vec4 v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  v = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  float x_38 = tint_symbol.y;
  if ((x_38 < 0.0f)) {
    float x_42 = func_();
    v = vec4(x_42, x_42, x_42, x_42);
  }
  if ((packUnorm4x8(v) == 1u)) {
    return;
  }
  uint x_50 = x_8.one;
  if (((1u << x_50) == 2u)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    int x_57 = x_10.x_GLF_uniform_int_values[0].el;
    float x_58 = float(x_57);
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
