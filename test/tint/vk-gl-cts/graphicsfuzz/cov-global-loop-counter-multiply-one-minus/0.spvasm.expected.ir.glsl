SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int x_23 = 0;
  int x_27 = 0;
  int x_37 = 0;
  int x_45 = 0;
  x_23 = 0;
  {
    while(true) {
      int x_24 = 0;
      x_27 = x_5.x_GLF_uniform_int_values[1].el;
      if ((x_23 < (100 - x_27))) {
      } else {
        break;
      }
      {
        x_24 = (x_23 + 1);
        x_23 = x_24;
      }
      continue;
    }
  }
  int x_40 = 0;
  int x_32 = x_5.x_GLF_uniform_int_values[0].el;
  x_45 = 1;
  if ((x_32 == 0)) {
    x_37 = 1;
    x_40 = x_23;
    {
      while(true) {
        int x_41 = 0;
        int x_38 = 0;
        if ((x_40 < 100)) {
        } else {
          break;
        }
        {
          x_41 = (x_40 + 1);
          x_38 = (x_37 * (1 - x_37));
          x_37 = x_38;
          x_40 = x_41;
        }
        continue;
      }
    }
    x_45 = x_37;
  }
  if ((x_45 == x_32)) {
    float x_50 = float(x_27);
    float x_51 = float(x_32);
    x_GLF_color = vec4(x_50, x_51, x_51, x_50);
  } else {
    x_GLF_color = vec4(float(x_32));
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
