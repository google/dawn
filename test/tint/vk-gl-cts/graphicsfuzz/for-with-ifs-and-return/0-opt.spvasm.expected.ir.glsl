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
  int i = 0;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 1;
  {
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      if ((tint_symbol.y < 0.0f)) {
        if ((tint_symbol.x < 0.0f)) {
          x_GLF_color = vec4(226.6959991455078125f, 1.0f, 1.0f, 1.0f);
        }
        {
          i = (i + 1);
        }
        continue;
      }
      return;
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
