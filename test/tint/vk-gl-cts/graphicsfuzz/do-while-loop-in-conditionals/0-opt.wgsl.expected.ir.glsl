SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  if (true) {
    float x_23 = tint_symbol.x;
    if ((x_23 < 0.0f)) {
      {
        while(true) {
          x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
          {
            float x_32 = tint_symbol.x;
            if (!((x_32 < 0.0f))) { break; }
          }
          continue;
        }
      }
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
