SKIP: FAILED

#version 310 es
precision mediump float;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  mat4x3 m43 = mat4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll1 = 0;
  int rows = 0;
  int ll4 = 0;
  int ll2 = 0;
  int c = 0;
  mat4x3 tempm43 = mat4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll3 = 0;
  int d = 0;
  int r = 0;
  float sums[9] = float[9](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int idx = 0;
  m43 = mat4x3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f));
  ll1 = 0;
  rows = 2;
  while (true) {
    if (true) {
    } else {
      break;
    }
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    if ((ll1 >= 5)) {
      break;
    }
    ll1 = (ll1 + 1);
    ll4 = 10;
    ll2 = 0;
    c = 0;
    {
      for(; (c < 1); c = (c + 1)) {
        if ((ll2 >= 0)) {
          break;
        }
        ll2 = (ll2 + 1);
        tempm43 = m43;
        ll3 = 0;
        d = 0;
        {
          for(; (1 < ll4); d = (d + 1)) {
            tempm43[(((d >= 0) & (d < 4)) ? d : 0)][(((r >= 0) & (r < 3)) ? r : 0)] = 1.0f;
          }
        }
        int x_111 = (((idx >= 0) & (idx < 9)) ? idx : 0);
        float x_113 = m43[c].y;
        float x_115 = sums[x_111];
        sums[x_111] = (x_115 + x_113);
      }
    }
    idx = (idx + 1);
  }
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
ERROR: 0:46: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:46: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



