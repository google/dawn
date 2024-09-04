SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[6];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 x_33 = vec4(0.0f);
  int x_36 = 0;
  int x_38 = 0;
  bool x_74 = false;
  bool x_75 = false;
  int x_29 = x_5.x_GLF_uniform_int_values[0].el;
  int x_31 = x_5.x_GLF_uniform_int_values[1].el;
  x_33 = vec4(0.0f);
  x_36 = x_29;
  x_38 = x_31;
  {
    while(true) {
      vec4 x_53 = vec4(0.0f);
      vec4 x_34 = vec4(0.0f);
      int x_62 = 0;
      int x_39 = 0;
      int x_41 = x_5.x_GLF_uniform_int_values[4].el;
      if ((x_38 < x_41)) {
      } else {
        break;
      }
      int x_56 = 0;
      switch(0u) {
        default:
        {
          if ((x_38 > x_5.x_GLF_uniform_int_values[3].el)) {
            x_34 = x_33;
            x_62 = 2;
            break;
          }
          x_53 = x_33;
          x_56 = x_29;
          {
            while(true) {
              vec4 x_54 = vec4(0.0f);
              int x_57 = 0;
              if ((x_56 < x_41)) {
              } else {
                break;
              }
              {
                x_54 = vec4(float((x_38 + x_56)));
                x_57 = (x_56 + 1);
                x_53 = x_54;
                x_56 = x_57;
              }
              continue;
            }
          }
          x_GLF_color = x_53;
          x_34 = x_53;
          x_62 = x_31;
          break;
        }
      }
      {
        x_39 = (x_38 + 1);
        x_33 = x_34;
        x_36 = (x_36 + x_62);
        x_38 = x_39;
      }
      continue;
    }
  }
  vec4 v = x_GLF_color;
  bool x_69 = all((v == vec4(float(x_5.x_GLF_uniform_int_values[2].el))));
  x_75 = x_69;
  if (x_69) {
    x_74 = (x_36 == x_5.x_GLF_uniform_int_values[5].el);
    x_75 = x_74;
  }
  if (x_75) {
    float x_79 = float(x_31);
    float x_80 = float(x_29);
    x_GLF_color = vec4(x_79, x_80, x_80, x_79);
  } else {
    x_GLF_color = vec4(float(x_29));
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
