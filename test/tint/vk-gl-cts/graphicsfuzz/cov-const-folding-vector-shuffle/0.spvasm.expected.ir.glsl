SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 x_26 = vec2(0.0f);
  int x_5 = 0;
  bool x_39 = false;
  bool x_40 = false;
  x_26 = vec2(0.0f);
  x_5 = 2;
  {
    while(true) {
      vec2 x_27 = vec2(0.0f);
      int x_4 = 0;
      if ((x_5 < 3)) {
      } else {
        break;
      }
      {
        x_27 = vec2(1.0f, float(x_5)).xy;
        x_4 = (x_5 + 1);
        x_26 = x_27;
        x_5 = x_4;
      }
      continue;
    }
  }
  bool x_34 = (x_26.x != 1.0f);
  x_40 = x_34;
  if (!(x_34)) {
    x_39 = (x_26.y != 2.0f);
    x_40 = x_39;
  }
  if (x_40) {
    x_GLF_color = vec4(0.0f);
  } else {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
