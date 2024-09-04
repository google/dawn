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


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int x_28 = 0;
  int x_31 = 0;
  int x_29 = 0;
  int x_42 = 0;
  int x_24 = (-2147483647 - 1);
  x_28 = x_5.x_GLF_uniform_int_values[3].el;
  x_31 = 1;
  {
    while(true) {
      int x_32 = 0;
      x_42 = x_28;
      if ((x_31 <= (x_24 - 1))) {
      } else {
        break;
      }
      x_29 = (x_28 + x_31);
      if ((x_5.x_GLF_uniform_int_values[0].el == 1)) {
        x_42 = x_29;
        break;
      }
      {
        x_32 = (x_31 + 1);
        x_28 = x_29;
        x_31 = x_32;
      }
      continue;
    }
  }
  if ((x_42 == x_5.x_GLF_uniform_int_values[2].el)) {
    float x_51 = float(x_5.x_GLF_uniform_int_values[0].el);
    float x_54 = float(x_5.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(x_51, x_54, x_54, x_51);
  } else {
    x_GLF_color = vec4(float(x_5.x_GLF_uniform_int_values[1].el));
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
