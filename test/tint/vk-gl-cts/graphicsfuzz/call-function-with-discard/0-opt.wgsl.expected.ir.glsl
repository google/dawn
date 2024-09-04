SKIP: FAILED

#version 310 es

struct buf0 {
  float one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void func_() {
  float x_28 = x_6.one;
  if ((1.0f > x_28)) {
    continue_execution = false;
  }
}
void main_1() {
  x_GLF_color = vec4(0.0f);
  {
    while(true) {
      func_();
      if (false) {
      } else {
        break;
      }
      {
      }
      continue;
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out main() {
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
