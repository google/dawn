SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v_1;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  vec4 v = vec4(0.0f);
  int i = 0;
  v = vec4(float(v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el));
  i = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el)) {
      } else {
        break;
      }
      int x_50 = i;
      v[uvec3(0u, 1u, 2u)[x_50]] = float(i);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  vec4 v_2 = v;
  float v_3 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el);
  float v_4 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el);
  float v_5 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[2].el);
  if (all((v_2 == vec4(v_3, v_4, v_5, float(v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el))))) {
    float v_6 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    float v_7 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    float v_8 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_6, v_7, v_8, float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el));
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
ERROR: 0:47: 'all' : no matching overloaded function found 
ERROR: 0:47: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
