SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
float fx_() {
  if ((tint_symbol.y >= 0.0f)) {
    float x_55 = x_7.injectionSwitch.y;
    return x_55;
  }
  {
    while(true) {
      if (true) {
      } else {
        break;
      }
      x_GLF_color = vec4(1.0f);
      {
      }
      continue;
    }
  }
  return 0.0f;
}
void main_1() {
  float x2 = 0.0f;
  float B = 0.0f;
  float k0 = 0.0f;
  x2 = 1.0f;
  B = 1.0f;
  float x_34 = fx_();
  x_GLF_color = vec4(x_34, 0.0f, 0.0f, 1.0f);
  {
    while(true) {
      if ((x2 > 2.0f)) {
      } else {
        break;
      }
      float x_43 = fx_();
      float x_44 = fx_();
      k0 = (x_43 - x_44);
      B = k0;
      x2 = B;
      {
      }
      continue;
    }
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
