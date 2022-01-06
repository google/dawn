SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
} x_5;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  mat2 m = mat2(0.0f, 0.0f, 0.0f, 0.0f);
  float f = 0.0f;
  int i = 0;
  int j = 0;
  int x_36 = x_5.x_GLF_uniform_int_values[1].el;
  if ((x_36 == 1)) {
    float x_40 = f;
    m = mat2(vec2(x_40, 0.0f), vec2(0.0f, x_40));
  }
  int x_45 = x_5.x_GLF_uniform_int_values[1].el;
  i = x_45;
  while (true) {
    int x_50 = i;
    int x_52 = x_5.x_GLF_uniform_int_values[0].el;
    if ((x_50 < x_52)) {
    } else {
      break;
    }
    int x_56 = x_5.x_GLF_uniform_int_values[1].el;
    j = x_56;
    while (true) {
      int x_61 = j;
      int x_63 = x_5.x_GLF_uniform_int_values[0].el;
      if ((x_61 < x_63)) {
      } else {
        break;
      }
      int x_66 = i;
      int x_67 = j;
      int x_68 = i;
      int x_70 = x_5.x_GLF_uniform_int_values[0].el;
      m[x_66][x_67] = float(((x_68 * x_70) + j));
      {
        j = (j + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
  mat2 x_80 = m;
  int x_82 = x_5.x_GLF_uniform_int_values[1].el;
  int x_85 = x_5.x_GLF_uniform_int_values[2].el;
  int x_88 = x_5.x_GLF_uniform_int_values[0].el;
  int x_91 = x_5.x_GLF_uniform_int_values[3].el;
  mat2 x_95 = mat2(vec2(float(x_82), float(x_85)), vec2(float(x_88), float(x_91)));
  if ((all(equal(x_80[0u], x_95[0u])) & all(equal(x_80[1u], x_95[1u])))) {
    int x_109 = x_5.x_GLF_uniform_int_values[2].el;
    int x_112 = x_5.x_GLF_uniform_int_values[1].el;
    int x_115 = x_5.x_GLF_uniform_int_values[1].el;
    int x_118 = x_5.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4(float(x_109), float(x_112), float(x_115), float(x_118));
  } else {
    int x_122 = x_5.x_GLF_uniform_int_values[1].el;
    float x_123 = float(x_122);
    x_GLF_color = vec4(x_123, x_123, x_123, x_123);
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
ERROR: 0:63: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' global bool' and a right operand of type ' global bool' (or there is no acceptable conversion)
ERROR: 0:63: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



