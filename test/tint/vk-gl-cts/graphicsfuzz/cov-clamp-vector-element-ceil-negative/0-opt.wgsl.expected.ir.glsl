SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_9;
void main_1() {
  vec2 v0 = vec2(0.0f);
  vec2 v1 = vec2(0.0f);
  float x_36 = x_6.x_GLF_uniform_float_values[1].el;
  v0 = vec2(x_36, -580.0150146484375f);
  vec2 x_38 = v0;
  float x_41 = x_6.x_GLF_uniform_float_values[0].el;
  float x_42 = -(x_41);
  vec2 v = ceil(x_38);
  v1 = clamp(v, vec2(x_42, x_42), vec2(100.0f));
  float x_46 = v1.y;
  float x_48 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_46 == -(x_48))) {
    int x_55 = x_9.x_GLF_uniform_int_values[0].el;
    int x_58 = x_9.x_GLF_uniform_int_values[1].el;
    int x_61 = x_9.x_GLF_uniform_int_values[1].el;
    int x_64 = x_9.x_GLF_uniform_int_values[0].el;
    float v_1 = float(x_55);
    float v_2 = float(x_58);
    float v_3 = float(x_61);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_64));
  } else {
    int x_68 = x_9.x_GLF_uniform_int_values[1].el;
    float x_69 = float(x_68);
    x_GLF_color = vec4(x_69, x_69, x_69, x_69);
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
