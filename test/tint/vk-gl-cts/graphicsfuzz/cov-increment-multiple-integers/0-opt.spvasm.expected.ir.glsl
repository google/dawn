SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
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
  int a = 0;
  int b = 0;
  int c = 0;
  bool x_76 = false;
  bool x_77 = false;
  bool x_83 = false;
  bool x_84 = false;
  a = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  b = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  c = 1;
  {
    while(true) {
      if (((b < v.tint_symbol_1.x_GLF_uniform_int_values[4].el) & (a < 10))) {
      } else {
        break;
      }
      if ((c > 5)) {
        break;
      }
      a = (a + 1);
      c = (c + 1);
      b = (b + 1);
      {
      }
      continue;
    }
  }
  {
    while(true) {
      if ((a < v.tint_symbol_1.x_GLF_uniform_int_values[1].el)) {
      } else {
        break;
      }
      {
        a = (a + 1);
      }
      continue;
    }
  }
  bool x_70 = (a == v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
  x_77 = x_70;
  if (x_70) {
    x_76 = (b == v.tint_symbol_1.x_GLF_uniform_int_values[3].el);
    x_77 = x_76;
  }
  x_84 = x_77;
  if (x_77) {
    x_83 = (c == v.tint_symbol_1.x_GLF_uniform_int_values[3].el);
    x_84 = x_83;
  }
  if (x_84) {
    float v_1 = float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
    float v_2 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    float v_3 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_1, v_2, v_3, float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el));
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
ERROR: 0:37: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:37: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
