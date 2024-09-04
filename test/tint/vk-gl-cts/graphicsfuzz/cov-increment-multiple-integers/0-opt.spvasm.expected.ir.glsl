SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
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
  int b = 0;
  int c = 0;
  bool x_76 = false;
  bool x_77 = false;
  bool x_83 = false;
  bool x_84 = false;
  a = x_6.x_GLF_uniform_int_values[0].el;
  b = x_6.x_GLF_uniform_int_values[2].el;
  c = 1;
  {
    while(true) {
      if (((b < x_6.x_GLF_uniform_int_values[4].el) & (a < 10))) {
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
      if ((a < x_6.x_GLF_uniform_int_values[1].el)) {
      } else {
        break;
      }
      {
        a = (a + 1);
      }
      continue;
    }
  }
  bool x_70 = (a == x_6.x_GLF_uniform_int_values[1].el);
  x_77 = x_70;
  if (x_70) {
    x_76 = (b == x_6.x_GLF_uniform_int_values[3].el);
    x_77 = x_76;
  }
  x_84 = x_77;
  if (x_77) {
    x_83 = (c == x_6.x_GLF_uniform_int_values[3].el);
    x_84 = x_83;
  }
  if (x_84) {
    float v = float(x_6.x_GLF_uniform_int_values[2].el);
    float v_1 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_6.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[0].el));
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
