SKIP: FAILED

#version 310 es

struct buf0 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
float func_() {
  float s = 0.0f;
  int i = 0;
  int j = 0;
  s = 2.0f;
  i = 0;
  {
    while(true) {
      if ((i < (x_8.zero + 1))) {
      } else {
        break;
      }
      s = (s + 3.0f);
      j = 0;
      {
        while(true) {
          if ((j < 10)) {
          } else {
            break;
          }
          if ((x_8.zero == 1)) {
            continue_execution = false;
          }
          {
            j = (j + 1);
          }
          continue;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float x_71 = s;
  return x_71;
}
void main_1() {
  vec4 c = vec4(0.0f);
  float x_34 = func_();
  c = vec4(x_34, 0.0f, 0.0f, 1.0f);
  float x_36 = func_();
  if ((x_36 == 5.0f)) {
    x_GLF_color = c;
  } else {
    x_GLF_color = vec4(0.0f);
  }
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
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
