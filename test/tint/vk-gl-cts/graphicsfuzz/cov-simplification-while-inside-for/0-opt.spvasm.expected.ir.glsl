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
      if ((i < x_6.one)) {
      } else {
        break;
      }
      {
        while(true) {
          if ((x_6.one == 1)) {
            x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
          }
          {
            if (true) { break; }
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
  v[1u] = float(x_9.zero);
  x_GLF_color[1u] = v.y;
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
