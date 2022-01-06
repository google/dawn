SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf1 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
};
struct tint_padded_array_element_1 {
  float el;
};
struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[1];
};

layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
} x_6;
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[1];
} x_10;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  mat4 m0 = mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int c = 0;
  mat4 m1 = mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int x_40 = x_6.x_GLF_uniform_int_values[1].el;
  float x_41 = float(x_40);
  m0 = mat4(vec4(x_41, 0.0f, 0.0f, 0.0f), vec4(0.0f, x_41, 0.0f, 0.0f), vec4(0.0f, 0.0f, x_41, 0.0f), vec4(0.0f, 0.0f, 0.0f, x_41));
  int x_48 = x_6.x_GLF_uniform_int_values[2].el;
  c = x_48;
  while (true) {
    int x_53 = c;
    int x_55 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_53 < x_55)) {
    } else {
      break;
    }
    m1 = m0;
    int x_59 = c;
    int x_61 = x_6.x_GLF_uniform_int_values[3].el;
    int x_64 = x_6.x_GLF_uniform_int_values[2].el;
    float x_66 = x_10.x_GLF_uniform_float_values[0].el;
    m1[(x_59 % x_61)][x_64] = x_66;
    int x_68 = c;
    int x_70 = x_6.x_GLF_uniform_int_values[3].el;
    int x_73 = x_6.x_GLF_uniform_int_values[2].el;
    float x_75 = x_10.x_GLF_uniform_float_values[0].el;
    m0[(x_68 % x_70)][x_73] = x_75;
    {
      c = (c + 1);
    }
  }
  mat4 x_79 = m0;
  int x_81 = x_6.x_GLF_uniform_int_values[1].el;
  int x_84 = x_6.x_GLF_uniform_int_values[2].el;
  int x_87 = x_6.x_GLF_uniform_int_values[1].el;
  int x_90 = x_6.x_GLF_uniform_int_values[1].el;
  int x_93 = x_6.x_GLF_uniform_int_values[1].el;
  int x_96 = x_6.x_GLF_uniform_int_values[2].el;
  int x_99 = x_6.x_GLF_uniform_int_values[1].el;
  int x_102 = x_6.x_GLF_uniform_int_values[1].el;
  int x_105 = x_6.x_GLF_uniform_int_values[1].el;
  int x_108 = x_6.x_GLF_uniform_int_values[2].el;
  int x_111 = x_6.x_GLF_uniform_int_values[1].el;
  int x_114 = x_6.x_GLF_uniform_int_values[1].el;
  int x_117 = x_6.x_GLF_uniform_int_values[1].el;
  int x_120 = x_6.x_GLF_uniform_int_values[2].el;
  int x_123 = x_6.x_GLF_uniform_int_values[1].el;
  int x_126 = x_6.x_GLF_uniform_int_values[1].el;
  mat4 x_132 = mat4(vec4(float(x_81), float(x_84), float(x_87), float(x_90)), vec4(float(x_93), float(x_96), float(x_99), float(x_102)), vec4(float(x_105), float(x_108), float(x_111), float(x_114)), vec4(float(x_117), float(x_120), float(x_123), float(x_126)));
  if ((((all(equal(x_79[0u], x_132[0u])) & all(equal(x_79[1u], x_132[1u]))) & all(equal(x_79[2u], x_132[2u]))) & all(equal(x_79[3u], x_132[3u])))) {
    int x_156 = x_6.x_GLF_uniform_int_values[2].el;
    int x_159 = x_6.x_GLF_uniform_int_values[1].el;
    int x_162 = x_6.x_GLF_uniform_int_values[1].el;
    int x_165 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4(float(x_156), float(x_159), float(x_162), float(x_165));
  } else {
    int x_169 = x_6.x_GLF_uniform_int_values[1].el;
    float x_170 = float(x_169);
    x_GLF_color = vec4(x_170, x_170, x_170, x_170);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_1 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(x_GLF_color);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:74: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' global bool' and a right operand of type ' global bool' (or there is no acceptable conversion)
ERROR: 0:74: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



