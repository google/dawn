SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v0 = vec4(0.0f);
  vec4 v1 = vec4(0.0f);
  int a = 0;
  int c = 0;
  v0 = vec4(x_6.x_GLF_uniform_float_values[1].el);
  v1 = vec4(x_6.x_GLF_uniform_float_values[0].el);
  a = x_10.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((a < x_10.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      c = x_10.x_GLF_uniform_int_values[3].el;
      {
        while(true) {
          if ((c < x_10.x_GLF_uniform_int_values[2].el)) {
          } else {
            break;
          }
          int x_69 = min(max(c, 0), 3);
          v0[x_69] = (v0[x_69] - x_6.x_GLF_uniform_float_values[1].el);
          if ((x_10.x_GLF_uniform_int_values[1].el == x_10.x_GLF_uniform_int_values[3].el)) {
            int x_83 = a;
            vec4 v = vec4(x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[1].el, 3.0f);
            vec4 v_1 = smoothstep(v, vec4(1.0f), v0);
            v1[x_83] = v_1[a];
          }
          {
            c = (c + 1);
          }
          continue;
        }
      }
      {
        a = (a + 1);
      }
      continue;
    }
  }
  if ((v1.x == x_6.x_GLF_uniform_float_values[0].el)) {
    float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_3 = float(x_10.x_GLF_uniform_int_values[3].el);
    float v_4 = float(x_10.x_GLF_uniform_int_values[3].el);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_10.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[3].el));
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
