SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct buf1 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_5;
uniform buf1 x_7;
bool continue_execution = true;
void main_1() {
  float v = float(x_5.x_GLF_uniform_int_values[0].el);
  float v_1 = float(x_5.x_GLF_uniform_int_values[1].el);
  float v_2 = float(x_5.x_GLF_uniform_int_values[1].el);
  x_GLF_color = vec4(v, v_1, v_2, float(x_5.x_GLF_uniform_int_values[0].el));
  {
    while(true) {
      if ((x_7.zero == x_5.x_GLF_uniform_int_values[0].el)) {
        {
          if (true) { break; }
        }
        continue;
      }
      if ((x_7.zero == x_5.x_GLF_uniform_int_values[2].el)) {
        break;
      }
      if ((x_7.zero == x_5.x_GLF_uniform_int_values[3].el)) {
        continue_execution = false;
      }
      return;
    }
  }
  x_GLF_color = vec4(float(x_5.x_GLF_uniform_int_values[1].el));
}
main_out main() {
  main_1();
  main_out v_3 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_3;
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
