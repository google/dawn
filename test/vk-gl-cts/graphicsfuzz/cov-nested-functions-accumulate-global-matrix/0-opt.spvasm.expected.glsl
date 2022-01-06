SKIP: FAILED

#version 310 es
precision mediump float;

struct buf2 {
  float one;
};
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
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
};

mat4x2 m = mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 2) uniform buf2_1 {
  float one;
} x_10;
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[1];
} x_12;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
} x_16;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void func0_i1_(inout int x) {
  int i = 0;
  bool x_137 = false;
  bool x_138 = false;
  bool x_138_phi = false;
  bool x_139_phi = false;
  int x_124 = x;
  bool x_125 = (x_124 < 1);
  x_139_phi = x_125;
  if (!(x_125)) {
    int x_129 = x;
    bool x_130 = (x_129 > 1);
    x_138_phi = x_130;
    if (x_130) {
      float x_134 = x_10.one;
      float x_136 = x_12.x_GLF_uniform_float_values[0].el;
      x_137 = (x_134 > x_136);
      x_138_phi = x_137;
    }
    x_138 = x_138_phi;
    x_139_phi = x_138;
  }
  if (x_139_phi) {
    return;
  }
  float x_143 = x_10.one;
  float x_145 = x_12.x_GLF_uniform_float_values[0].el;
  if ((x_143 == x_145)) {
    i = 0;
    while (true) {
      if ((i < 2)) {
      } else {
        break;
      }
      {
        int x_154 = x;
        int x_155 = clamp(x_154, 0, 3);
        int x_156 = i;
        float x_158 = x_12.x_GLF_uniform_float_values[0].el;
        float x_160 = m[x_155][x_156];
        m[x_155][x_156] = (x_160 + x_158);
        i = (i + 1);
      }
    }
  }
  return;
}

void func1_() {
  int param = 0;
  float x_167 = tint_symbol.y;
  if ((x_167 < 0.0f)) {
    return;
  }
  param = 1;
  func0_i1_(param);
  return;
}

void main_1() {
  m = mat4x2(vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f));
  func1_();
  func1_();
  mat4x2 x_54 = m;
  int x_56 = x_16.x_GLF_uniform_int_values[0].el;
  int x_59 = x_16.x_GLF_uniform_int_values[0].el;
  int x_62 = x_16.x_GLF_uniform_int_values[1].el;
  int x_65 = x_16.x_GLF_uniform_int_values[1].el;
  int x_68 = x_16.x_GLF_uniform_int_values[0].el;
  int x_71 = x_16.x_GLF_uniform_int_values[0].el;
  int x_74 = x_16.x_GLF_uniform_int_values[0].el;
  int x_77 = x_16.x_GLF_uniform_int_values[0].el;
  mat4x2 x_83 = mat4x2(vec2(float(x_56), float(x_59)), vec2(float(x_62), float(x_65)), vec2(float(x_68), float(x_71)), vec2(float(x_74), float(x_77)));
  if ((((all(equal(x_54[0u], x_83[0u])) & all(equal(x_54[1u], x_83[1u]))) & all(equal(x_54[2u], x_83[2u]))) & all(equal(x_54[3u], x_83[3u])))) {
    int x_107 = x_16.x_GLF_uniform_int_values[3].el;
    int x_110 = x_16.x_GLF_uniform_int_values[0].el;
    int x_113 = x_16.x_GLF_uniform_int_values[0].el;
    int x_116 = x_16.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4(float(x_107), float(x_110), float(x_113), float(x_116));
  } else {
    int x_120 = x_16.x_GLF_uniform_int_values[0].el;
    float x_121 = float(x_120);
    x_GLF_color = vec4(x_121, x_121, x_121, x_121);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_4 {
  vec4 tint_symbol_2;
};
struct tint_symbol_5 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_6 = main_out(x_GLF_color);
  return tint_symbol_6;
}

tint_symbol_5 tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  main_out inner_result = tint_symbol_1_inner(tint_symbol_3.tint_symbol_2);
  tint_symbol_5 wrapper_result = tint_symbol_5(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_4 inputs;
  inputs.tint_symbol_2 = gl_FragCoord;
  tint_symbol_5 outputs;
  outputs = tint_symbol_1(inputs);
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:106: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' global bool' and a right operand of type ' global bool' (or there is no acceptable conversion)
ERROR: 0:106: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



