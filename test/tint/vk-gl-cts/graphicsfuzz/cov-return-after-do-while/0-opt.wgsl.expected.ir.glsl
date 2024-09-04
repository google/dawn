SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_5;
void main_1() {
  int x_22 = x_5.x_GLF_uniform_int_values[0].el;
  int x_25 = x_5.x_GLF_uniform_int_values[1].el;
  int x_28 = x_5.x_GLF_uniform_int_values[1].el;
  int x_31 = x_5.x_GLF_uniform_int_values[0].el;
  float v = float(x_22);
  float v_1 = float(x_25);
  float v_2 = float(x_28);
  x_GLF_color = vec4(v, v_1, v_2, float(x_31));
  int x_35 = x_5.x_GLF_uniform_int_values[1].el;
  int x_37 = x_5.x_GLF_uniform_int_values[0].el;
  if ((x_35 > x_37)) {
    {
      while(true) {
        int x_46 = x_5.x_GLF_uniform_int_values[0].el;
        float x_47 = float(x_46);
        x_GLF_color = vec4(x_47, x_47, x_47, x_47);
        {
          int x_50 = x_5.x_GLF_uniform_int_values[1].el;
          int x_52 = x_5.x_GLF_uniform_int_values[0].el;
          if (!((x_50 > x_52))) { break; }
        }
        continue;
      }
    }
    return;
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
