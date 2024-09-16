SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};

int x_GLF_global_loop_count = 0;
layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
int func_() {
  {
    while(true) {
      int x_72 = x_GLF_global_loop_count;
      if ((x_72 < 100)) {
      } else {
        break;
      }
      int x_75 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_75 + 1);
      int x_78 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
      return x_78;
    }
  }
  int x_80 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  return x_80;
}
void main_1() {
  int a = 0;
  x_GLF_global_loop_count = 0;
  {
    while(true) {
      int x_35 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_35 + 1);
      if (false) {
        return;
      }
      {
        int x_39 = x_GLF_global_loop_count;
        if (!((true & (x_39 < 100)))) { break; }
      }
      continue;
    }
  }
  int x_42 = func_();
  a = x_42;
  int x_43 = a;
  int x_45 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  if ((x_43 == x_45)) {
    int x_51 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    int x_54 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_57 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_60 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    float v_1 = float(x_51);
    float v_2 = float(x_54);
    float v_3 = float(x_57);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_60));
  } else {
    int x_64 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float x_65 = float(x_64);
    x_GLF_color = vec4(x_65, x_65, x_65, x_65);
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
ERROR: 0:54: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' const bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:54: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
