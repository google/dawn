SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v = vec4(0.0f);
  int i = 0;
  float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[2].el);
  float v_3 = float(x_6.x_GLF_uniform_int_values[3].el);
  v = vec4(v_1, v_2, v_3, float(x_6.x_GLF_uniform_int_values[0].el));
  i = x_6.x_GLF_uniform_int_values[4].el;
  {
    while(true) {
      if ((i < x_6.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      vec4 v_4 = vec4(v.x, v.y, v.z, v.w);
      vec4 v_5 = vec4(v.x, v.y, v.z, v.w);
      vec4 v_6 = vec4(v.x, v.y, v.z, v.w);
      mat4 v_7 = mat4(v_4, v_5, v_6, vec4(v.x, v.y, v.z, v.w));
      if ((v_7[0u][i] > x_9.x_GLF_uniform_float_values[0].el)) {
        int x_96 = i;
        vec4 v_8 = v;
        vec4 v_9 = vec4(x_9.x_GLF_uniform_float_values[1].el);
        vec4 v_10 = clamp(v_8, v_9, vec4(x_9.x_GLF_uniform_float_values[0].el));
        v[x_96] = v_10[x_6.x_GLF_uniform_int_values[1].el];
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  vec4 v_11 = v;
  if (all((v_11 == vec4(float(x_6.x_GLF_uniform_int_values[1].el))))) {
    float v_12 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_13 = float(x_6.x_GLF_uniform_int_values[4].el);
    float v_14 = float(x_6.x_GLF_uniform_int_values[4].el);
    x_GLF_color = vec4(v_12, v_13, v_14, float(x_6.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[4].el));
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
