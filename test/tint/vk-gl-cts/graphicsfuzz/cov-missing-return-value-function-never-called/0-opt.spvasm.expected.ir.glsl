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
  if ((tint_symbol.y < 0.0f)) {
    float x_42 = func_();
    v = vec4(x_42);
  }
  if ((packUnorm4x8(v) == 1u)) {
    return;
  }
  if (((1u << (x_8.one & 31u)) == 2u)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[0].el));
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
