SKIP: FAILED

#version 310 es

struct buf0 {
  int one;
};

struct buf1 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
uniform buf1 x_9;
void main_1() {
  int i = 0;
  vec4 v = vec4(0.0f);
  x_GLF_color = vec4(0.0f);
  i = 0;
  {
    while(true) {
      int x_38 = i;
      int x_40 = x_6.one;
      if ((x_38 < x_40)) {
      } else {
        break;
      }
      {
        while(true) {
          int x_48 = x_6.one;
          if ((x_48 == 1)) {
            x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
          }
          {
            if (true) { break; }
          }
          continue;
        }
      }
      {
        int x_52 = i;
        i = (x_52 + 1);
      }
      continue;
    }
  }
  int x_55 = x_9.zero;
  v[1u] = float(x_55);
  float x_59 = v.y;
  x_GLF_color[1u] = x_59;
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
