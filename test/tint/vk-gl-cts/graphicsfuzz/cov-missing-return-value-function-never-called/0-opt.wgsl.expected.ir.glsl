SKIP: FAILED

#version 310 es

struct buf1 {
  uint one;
};

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_10;
float func_() {
  switch(1) {
    case 0:
    {
      return 1.0f;
    }
    default:
    {
      break;
    }
  }
  return 0.0f;
}
void main_1() {
  vec4 v = vec4(0.0f);
  v = vec4(1.0f);
  float x_38 = tint_symbol.y;
  if ((x_38 < 0.0f)) {
    float x_42 = func_();
    v = vec4(x_42, x_42, x_42, x_42);
  }
  vec4 x_44 = v;
  if ((packUnorm4x8(x_44) == 1u)) {
    return;
  }
  uint x_50 = x_8.one;
  if (((1u << (x_50 & 31u)) == 2u)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    int x_57 = x_10.x_GLF_uniform_int_values[0].el;
    float x_58 = float(x_57);
    x_GLF_color = vec4(x_58, x_58, x_58, x_58);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
