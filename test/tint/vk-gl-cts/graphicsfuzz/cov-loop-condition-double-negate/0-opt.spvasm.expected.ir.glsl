SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[6];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  int arr[3] = int[3](0, 0, 0);
  int index = 0;
  bool x_76 = false;
  bool x_77 = false;
  bool x_86 = false;
  bool x_87 = false;
  arr = int[3](v.tint_symbol_1.x_GLF_uniform_int_values[3].el, v.tint_symbol_1.x_GLF_uniform_int_values[5].el, v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
  index = 1;
  {
    while(true) {
      bool x_51 = false;
      bool x_52 = false;
      x_52 = true;
      if (true) {
        x_51 = !(((v.tint_symbol_1.x_GLF_uniform_int_values[0].el == 1) & (index <= 1)));
        x_52 = x_51;
      }
      if (!(x_52)) {
      } else {
        break;
      }
      int x_55 = index;
      int x_56_save = x_55;
      arr[x_56_save] = (arr[x_55] + 1);
      index = (index + 1);
      {
      }
      continue;
    }
  }
  bool x_67 = (arr[v.tint_symbol_1.x_GLF_uniform_int_values[1].el] == v.tint_symbol_1.x_GLF_uniform_int_values[3].el);
  x_77 = x_67;
  if (x_67) {
    x_76 = (arr[v.tint_symbol_1.x_GLF_uniform_int_values[0].el] == v.tint_symbol_1.x_GLF_uniform_int_values[4].el);
    x_77 = x_76;
  }
  x_87 = x_77;
  if (x_77) {
    x_86 = (arr[v.tint_symbol_1.x_GLF_uniform_int_values[3].el] == v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
    x_87 = x_86;
  }
  if (x_87) {
    float v_1 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    float v_2 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    float v_3 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_1, v_2, v_3, float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el));
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:39: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:39: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
