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
  bool x_39 = false;
  vec2 x_26_phi = vec2(0.0f);
  int x_5_phi = 0;
  bool x_40_phi = false;
  x_26_phi = vec2(0.0f);
  x_5_phi = 2;
  {
    while(true) {
      vec2 x_27 = vec2(0.0f);
      int x_4 = 0;
      x_26 = x_26_phi;
      int x_5 = x_5_phi;
      if ((x_5 < 3)) {
      } else {
        break;
      }
      {
        vec2 x_32 = vec2(1.0f, float(x_5));
        x_27 = vec2(x_32[0u], x_32[1u]);
        x_4 = (x_5 + 1);
        x_26_phi = x_27;
        x_5_phi = x_4;
      }
      continue;
    }
  }
  bool x_34 = (x_26.x != 1.0f);
  x_40_phi = x_34;
  if (!(x_34)) {
    x_39 = (x_26.y != 2.0f);
    x_40_phi = x_39;
  }
  bool x_40 = x_40_phi;
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
