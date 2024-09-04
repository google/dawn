SKIP: FAILED

#version 310 es

struct buf1 {
  vec2 v1;
};

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


uniform buf1 x_5;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_7;
void main_1() {
  float x_29 = x_5.v1.x;
  float x_31 = x_5.v1.y;
  if ((x_29 < x_31)) {
    int x_37 = x_7.x_GLF_uniform_int_values[0].el;
    int x_40 = x_7.x_GLF_uniform_int_values[1].el;
    int x_43 = x_7.x_GLF_uniform_int_values[1].el;
    int x_46 = x_7.x_GLF_uniform_int_values[0].el;
    float v = float(x_37);
    float v_1 = float(x_40);
    float v_2 = float(x_43);
    x_GLF_color = vec4(v, v_1, v_2, float(x_46));
    float x_50 = x_5.v1.x;
    float x_52 = x_5.v1.y;
    if ((x_50 > x_52)) {
      int x_57 = x_7.x_GLF_uniform_int_values[0].el;
      float x_58 = float(x_57);
      x_GLF_color = vec4(x_58, x_58, x_58, x_58);
    }
    return;
  } else {
    int x_61 = x_7.x_GLF_uniform_int_values[1].el;
    float x_62 = float(x_61);
    x_GLF_color = vec4(x_62, x_62, x_62, x_62);
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
