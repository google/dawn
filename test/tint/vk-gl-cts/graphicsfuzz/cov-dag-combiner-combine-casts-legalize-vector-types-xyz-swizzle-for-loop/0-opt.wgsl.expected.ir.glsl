SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v = vec4(0.0f);
  int i = 0;
  int x_36 = x_6.x_GLF_uniform_int_values[3].el;
  float x_37 = float(x_36);
  v = vec4(x_37, x_37, x_37, x_37);
  int x_40 = x_6.x_GLF_uniform_int_values[0].el;
  i = x_40;
  {
    while(true) {
      int x_45 = i;
      int x_47 = x_6.x_GLF_uniform_int_values[3].el;
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
  int x_59 = x_6.x_GLF_uniform_int_values[0].el;
  int x_62 = x_6.x_GLF_uniform_int_values[1].el;
  int x_65 = x_6.x_GLF_uniform_int_values[2].el;
  int x_68 = x_6.x_GLF_uniform_int_values[3].el;
  float v_1 = float(x_59);
  float v_2 = float(x_62);
  float v_3 = float(x_65);
  if (all((x_57 == vec4(v_1, v_2, v_3, float(x_68))))) {
    int x_77 = x_6.x_GLF_uniform_int_values[1].el;
    int x_80 = x_6.x_GLF_uniform_int_values[0].el;
    int x_83 = x_6.x_GLF_uniform_int_values[0].el;
    int x_86 = x_6.x_GLF_uniform_int_values[1].el;
    float v_4 = float(x_77);
    float v_5 = float(x_80);
    float v_6 = float(x_83);
    x_GLF_color = vec4(v_4, v_5, v_6, float(x_86));
  } else {
    int x_90 = x_6.x_GLF_uniform_int_values[0].el;
    float x_91 = float(x_90);
    x_GLF_color = vec4(x_91, x_91, x_91, x_91);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
