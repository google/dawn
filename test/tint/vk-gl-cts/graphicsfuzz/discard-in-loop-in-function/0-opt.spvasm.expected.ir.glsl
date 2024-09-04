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


uniform buf0 x_7;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void f_() {
  {
    while(true) {
      if ((1.0f > x_7.injectionSwitch.y)) {
        if ((tint_symbol.y < 0.0f)) {
          {
            if (true) { break; }
          }
          continue;
        } else {
          {
            if (true) { break; }
          }
          continue;
        }
      }
      continue_execution = false;
      {
        if (true) { break; }
      }
      continue;
    }
  }
}
void main_1() {
  f_();
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
