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
      if ((x_GLF_global_loop_count < 100)) {
      } else {
        break;
      }
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
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
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
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
  if ((a == v.tint_symbol_1.x_GLF_uniform_int_values[2].el)) {
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
ERROR: 0:51: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' const bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:51: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
