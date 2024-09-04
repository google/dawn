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


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v = vec4(0.0f);
  int i = 0;
  v = vec4(float(x_6.x_GLF_uniform_int_values[3].el));
  i = x_6.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < x_6.x_GLF_uniform_int_values[3].el)) {
      } else {
        break;
      }
      int x_50 = i;
      v[uvec3(0u, 1u, 2u)[x_50]] = float(i);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  vec4 v_1 = v;
  float v_2 = float(x_6.x_GLF_uniform_int_values[0].el);
  float v_3 = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_4 = float(x_6.x_GLF_uniform_int_values[2].el);
  if (all((v_1 == vec4(v_2, v_3, v_4, float(x_6.x_GLF_uniform_int_values[3].el))))) {
    float v_5 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_6 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_7 = float(x_6.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_5, v_6, v_7, float(x_6.x_GLF_uniform_int_values[1].el));
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
