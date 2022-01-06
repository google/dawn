SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  int injected;
};

layout (binding = 0) uniform buf0_1 {
  int injected;
} x_9;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int idx = 0;
  mat4x3 m43 = mat4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_1 = 0;
  int GLF_live6rows = 0;
  int z = 0;
  int ll_2 = 0;
  int ctr = 0;
  mat4x3 tempm43 = mat4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_3 = 0;
  int c = 0;
  int d = 0;
  float GLF_live6sums[9] = float[9](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  idx = 0;
  m43 = mat4x3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f));
  ll_1 = 0;
  GLF_live6rows = 2;
  while (true) {
    int x_18 = ll_1;
    int x_19 = x_9.injected;
    if ((x_18 >= x_19)) {
      x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
    }
    ll_1 = (ll_1 + 1);
    int x_22 = x_9.injected;
    z = x_22;
    ll_2 = 0;
    ctr = 0;
    {
      for(; (ctr < 1); ctr = (ctr + 1)) {
        int x_24 = ll_2;
        int x_25 = x_9.injected;
        if ((x_24 >= x_25)) {
          break;
        }
        ll_2 = (ll_2 + 1);
        tempm43 = m43;
        ll_3 = 0;
        c = 0;
        {
          for(; (1 < z); c = (c + 1)) {
            d = 0;
            tempm43[(((c >= 0) & (c < 4)) ? c : 0)][(((d >= 0) & (d < 3)) ? d : 0)] = 1.0f;
          }
        }
        int x_117 = (((idx >= 0) & (idx < 9)) ? idx : 0);
        float x_119 = m43[ctr].y;
        float x_121 = GLF_live6sums[x_117];
        GLF_live6sums[x_117] = (x_121 + x_119);
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
ERROR: 0:56: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:56: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



