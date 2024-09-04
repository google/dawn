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
      int x_47 = i;
      int x_49 = x_8.zero;
      if ((x_47 < (x_49 + 1))) {
      } else {
        break;
      }
      float x_53 = s;
      s = (x_53 + 3.0f);
      j = 0;
      {
        while(true) {
          int x_59 = j;
          if ((x_59 < 10)) {
          } else {
            break;
          }
          int x_63 = x_8.zero;
          if ((x_63 == 1)) {
            continue_execution = false;
          }
          {
            int x_67 = j;
            j = (x_67 + 1);
          }
          continue;
        }
      }
      {
        int x_69 = i;
        i = (x_69 + 1);
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
    vec4 x_41 = c;
    x_GLF_color = x_41;
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
