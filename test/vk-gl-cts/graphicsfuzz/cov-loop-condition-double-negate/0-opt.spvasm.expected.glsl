SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[6];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[6];
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int arr[3] = int[3](0, 0, 0);
  int index = 0;
  bool x_76 = false;
  bool x_86 = false;
  bool x_77_phi = false;
  bool x_87_phi = false;
  int x_33 = x_6.x_GLF_uniform_int_values[3].el;
  int x_35 = x_6.x_GLF_uniform_int_values[5].el;
  int x_37 = x_6.x_GLF_uniform_int_values[2].el;
  int tint_symbol_2[3] = int[3](x_33, x_35, x_37);
  arr = tint_symbol_2;
  index = 1;
  while (true) {
    bool x_51 = false;
    bool x_52_phi = false;
    x_52_phi = true;
    if (true) {
      int x_46 = x_6.x_GLF_uniform_int_values[0].el;
      x_51 = !(((x_46 == 1) & (index <= 1)));
      x_52_phi = x_51;
    }
    if (!(x_52_phi)) {
    } else {
      break;
    }
    int x_56_save = index;
    int x_57 = arr[x_56_save];
    arr[x_56_save] = (x_57 + 1);
    index = (index + 1);
  }
  int x_62 = x_6.x_GLF_uniform_int_values[1].el;
  int x_64 = arr[x_62];
  int x_66 = x_6.x_GLF_uniform_int_values[3].el;
  bool x_67 = (x_64 == x_66);
  x_77_phi = x_67;
  if (x_67) {
    int x_71 = x_6.x_GLF_uniform_int_values[0].el;
    int x_73 = arr[x_71];
    int x_75 = x_6.x_GLF_uniform_int_values[4].el;
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  bool x_77 = x_77_phi;
  x_87_phi = x_77;
  if (x_77) {
    int x_81 = x_6.x_GLF_uniform_int_values[3].el;
    int x_83 = arr[x_81];
    int x_85 = x_6.x_GLF_uniform_int_values[2].el;
    x_86 = (x_83 == x_85);
    x_87_phi = x_86;
  }
  if (x_87_phi) {
    int x_92 = x_6.x_GLF_uniform_int_values[0].el;
    int x_95 = x_6.x_GLF_uniform_int_values[1].el;
    int x_98 = x_6.x_GLF_uniform_int_values[1].el;
    int x_101 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_92), float(x_95), float(x_98), float(x_101));
  } else {
    int x_105 = x_6.x_GLF_uniform_int_values[1].el;
    float x_106 = float(x_105);
    x_GLF_color = vec4(x_106, x_106, x_106, x_106);
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
  main_out tint_symbol_3 = main_out(x_GLF_color);
  return tint_symbol_3;
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
ERROR: 0:35: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:35: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



