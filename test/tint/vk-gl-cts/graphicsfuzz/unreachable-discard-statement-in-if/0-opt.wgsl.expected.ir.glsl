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
vec3 computePoint_() {
  float x_48 = x_7.injectionSwitch.x;
  float x_50 = x_7.injectionSwitch.y;
  if ((x_48 > x_50)) {
    continue_execution = false;
  }
  return vec3(0.0f);
}
void main_1() {
  bool x_34 = false;
  {
    while(true) {
      vec3 x_36 = computePoint_();
      float x_41 = tint_symbol.x;
      if ((x_41 < 0.0f)) {
        x_34 = true;
        break;
      }
      vec3 x_45 = computePoint_();
      x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      x_34 = true;
      break;
    }
  }
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
