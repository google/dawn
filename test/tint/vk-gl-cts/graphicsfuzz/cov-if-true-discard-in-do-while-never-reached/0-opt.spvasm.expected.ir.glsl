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


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  int a = 0;
  a = 1;
  {
    while(true) {
      if ((a >= x_6.x_GLF_uniform_int_values[0].el)) {
        break;
      }
      if (true) {
        continue_execution = false;
      }
      a = (a + 1);
      {
        int x_39 = a;
        if (!((x_39 != 1))) { break; }
      }
      continue;
    }
  }
  if ((a == 1)) {
    float v = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_6.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(1.0f, v, v_1, float(x_6.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[0].el));
  }
}
main_out main() {
  main_1();
  main_out v_2 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_2;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
