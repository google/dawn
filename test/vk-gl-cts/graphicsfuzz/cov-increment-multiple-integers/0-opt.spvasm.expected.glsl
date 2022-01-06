SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[5];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[5];
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int b = 0;
  int c = 0;
  bool x_76 = false;
  bool x_83 = false;
  bool x_77_phi = false;
  bool x_84_phi = false;
  int x_31 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_31;
  int x_33 = x_6.x_GLF_uniform_int_values[2].el;
  b = x_33;
  c = 1;
  while (true) {
    int x_38 = b;
    int x_40 = x_6.x_GLF_uniform_int_values[4].el;
    if (((x_38 < x_40) & (a < 10))) {
    } else {
      break;
    }
    if ((c > 5)) {
      break;
    }
    a = (a + 1);
    c = (c + 1);
    b = (b + 1);
  }
  while (true) {
    int x_60 = a;
    int x_62 = x_6.x_GLF_uniform_int_values[1].el;
    if ((x_60 < x_62)) {
    } else {
      break;
    }
    {
      a = (a + 1);
    }
  }
  int x_67 = a;
  int x_69 = x_6.x_GLF_uniform_int_values[1].el;
  bool x_70 = (x_67 == x_69);
  x_77_phi = x_70;
  if (x_70) {
    int x_73 = b;
    int x_75 = x_6.x_GLF_uniform_int_values[3].el;
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  bool x_77 = x_77_phi;
  x_84_phi = x_77;
  if (x_77) {
    int x_80 = c;
    int x_82 = x_6.x_GLF_uniform_int_values[3].el;
    x_83 = (x_80 == x_82);
    x_84_phi = x_83;
  }
  if (x_84_phi) {
    int x_89 = x_6.x_GLF_uniform_int_values[2].el;
    int x_92 = x_6.x_GLF_uniform_int_values[0].el;
    int x_95 = x_6.x_GLF_uniform_int_values[0].el;
    int x_98 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4(float(x_89), float(x_92), float(x_95), float(x_98));
  } else {
    int x_102 = x_6.x_GLF_uniform_int_values[0].el;
    float x_103 = float(x_102);
    x_GLF_color = vec4(x_103, x_103, x_103, x_103);
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
ERROR: 0:32: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:32: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



