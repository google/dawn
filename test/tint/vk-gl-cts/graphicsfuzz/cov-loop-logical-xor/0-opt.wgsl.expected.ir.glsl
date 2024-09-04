SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int x_26 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_26;
  int x_28 = x_6.x_GLF_uniform_int_values[1].el;
  float x_29 = float(x_28);
  x_GLF_color = vec4(x_29, x_29, x_29, x_29);
  {
    while(true) {
      int x_36 = x_6.x_GLF_uniform_int_values[2].el;
      int x_37 = a;
      if (((x_36 == x_37) != true)) {
      } else {
        break;
      }
      int x_42 = x_6.x_GLF_uniform_int_values[0].el;
      int x_45 = x_6.x_GLF_uniform_int_values[1].el;
      int x_48 = x_6.x_GLF_uniform_int_values[1].el;
      int x_51 = x_6.x_GLF_uniform_int_values[0].el;
      float v = float(x_42);
      float v_1 = float(x_45);
      float v_2 = float(x_48);
      x_GLF_color = vec4(v, v_1, v_2, float(x_51));
      break;
    }
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
