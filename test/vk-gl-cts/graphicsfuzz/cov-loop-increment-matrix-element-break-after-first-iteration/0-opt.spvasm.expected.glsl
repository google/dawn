SKIP: FAILED

#version 310 es
precision mediump float;

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
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
};

layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
} x_7;
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
} x_10;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  mat2x3 m23 = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  float x_46 = x_7.x_GLF_uniform_float_values[1].el;
  m23 = mat2x3(vec3(x_46, 0.0f, 0.0f), vec3(0.0f, x_46, 0.0f));
  i = 1;
  while (true) {
    bool x_80 = false;
    bool x_81_phi = false;
    int x_54 = i;
    int x_56 = x_10.x_GLF_uniform_int_values[3].el;
    if ((x_54 < x_56)) {
    } else {
      break;
    }
    int x_60 = x_10.x_GLF_uniform_int_values[0].el;
    int x_62 = x_10.x_GLF_uniform_int_values[2].el;
    float x_64 = x_7.x_GLF_uniform_float_values[0].el;
    float x_66 = m23[x_60][x_62];
    m23[x_60][x_62] = (x_66 + x_64);
    float x_70 = tint_symbol.y;
    float x_72 = x_7.x_GLF_uniform_float_values[0].el;
    if ((x_70 < x_72)) {
    }
    x_81_phi = true;
    if (true) {
      float x_79 = tint_symbol.x;
      x_80 = (x_79 < 0.0f);
      x_81_phi = x_80;
    }
    if (!(x_81_phi)) {
      break;
    }
    {
      i = (i + 1);
    }
  }
  mat2x3 x_87 = m23;
  int x_89 = x_10.x_GLF_uniform_int_values[1].el;
  int x_92 = x_10.x_GLF_uniform_int_values[1].el;
  int x_95 = x_10.x_GLF_uniform_int_values[1].el;
  int x_98 = x_10.x_GLF_uniform_int_values[1].el;
  int x_101 = x_10.x_GLF_uniform_int_values[1].el;
  int x_104 = x_10.x_GLF_uniform_int_values[0].el;
  mat2x3 x_108 = mat2x3(vec3(float(x_89), float(x_92), float(x_95)), vec3(float(x_98), float(x_101), float(x_104)));
  if ((all(equal(x_87[0u], x_108[0u])) & all(equal(x_87[1u], x_108[1u])))) {
    int x_122 = x_10.x_GLF_uniform_int_values[0].el;
    int x_125 = x_10.x_GLF_uniform_int_values[1].el;
    int x_128 = x_10.x_GLF_uniform_int_values[1].el;
    int x_131 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_122), float(x_125), float(x_128), float(x_131));
  } else {
    int x_135 = x_10.x_GLF_uniform_int_values[1].el;
    float x_136 = float(x_135);
    x_GLF_color = vec4(x_136, x_136, x_136, x_136);
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
ERROR: 0:71: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' global bool' and a right operand of type ' global bool' (or there is no acceptable conversion)
ERROR: 0:71: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



