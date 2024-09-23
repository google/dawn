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
  int x_36 = v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  float x_37 = float(x_36);
  v = vec4(x_37, x_37, x_37, x_37);
  int x_40 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  i = x_40;
  {
    while(true) {
      int x_45 = i;
      int x_47 = v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el;
      if ((x_45 < x_47)) {
      } else {
        break;
      }
      int x_50 = i;
      int x_51 = i;
      v[uvec3(0u, 1u, 2u)[x_50]] = float(x_51);
      {
        int x_55 = i;
        i = (x_55 + 1);
      }
      continue;
    }
  }
  vec4 x_57 = v;
  int x_59 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  int x_62 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_65 = v_1.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_68 = v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  float v_2 = float(x_59);
  float v_3 = float(x_62);
  float v_4 = float(x_65);
  if (all((x_57 == vec4(v_2, v_3, v_4, float(x_68))))) {
    int x_77 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_80 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    int x_83 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    int x_86 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float v_5 = float(x_77);
    float v_6 = float(x_80);
    float v_7 = float(x_83);
    x_GLF_color = vec4(v_5, v_6, v_7, float(x_86));
  } else {
    int x_90 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    float x_91 = float(x_90);
    x_GLF_color = vec4(x_91, x_91, x_91, x_91);
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
ERROR: 0:58: 'all' : no matching overloaded function found 
ERROR: 0:58: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
