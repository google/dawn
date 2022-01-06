SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 injectionSwitch;
};

layout (binding = 0) uniform buf0_1 {
  vec2 injectionSwitch;
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int GLF_dead5cols = 0;
  int GLF_dead5rows = 0;
  int GLF_dead5c = 0;
  int GLF_dead5r = 0;
  int msb10 = 0;
  float donor_replacementGLF_dead5sums[9] = float[9](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  i = 0;
  while (true) {
    int x_45 = i;
    float x_47 = x_6.injectionSwitch.x;
    if ((x_45 >= int(x_47))) {
      break;
    }
    float x_53 = x_6.injectionSwitch.y;
    if ((0.0f > x_53)) {
      GLF_dead5cols = 2;
      {
        for(; (GLF_dead5cols <= 4); GLF_dead5cols = (GLF_dead5cols + 1)) {
          GLF_dead5rows = 2;
          {
            for(; (GLF_dead5rows <= 4); GLF_dead5rows = (GLF_dead5rows + 1)) {
              GLF_dead5c = 0;
              {
                for(; (GLF_dead5c < GLF_dead5cols); GLF_dead5c = (GLF_dead5c + 1)) {
                  GLF_dead5r = 0;
                  {
                    for(; (GLF_dead5r < GLF_dead5rows); GLF_dead5r = (GLF_dead5r + 1)) {
                      switch(msb10) {
                        case 1:
                        case 8: {
                          int x_96 = (((msb10 >= 0) & (msb10 < 9)) ? msb10 : 0);
                          float x_98 = donor_replacementGLF_dead5sums[x_96];
                          donor_replacementGLF_dead5sums[x_96] = (x_98 + 1.0f);
                          break;
                        }
                        default: {
                          break;
                        }
                      }
                    }
                  }
                }
              }
              msb10 = (msb10 + 1);
            }
          }
        }
      }
    }
    i = (i + 1);
    {
      if ((i < 200)) {
      } else {
        break;
      }
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_1 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(x_GLF_color);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:45: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:45: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



