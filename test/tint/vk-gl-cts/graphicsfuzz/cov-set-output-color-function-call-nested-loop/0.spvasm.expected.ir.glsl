SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[6];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  vec4 x_33 = vec4(0.0f);
  int x_36 = 0;
  int x_38 = 0;
  bool x_74 = false;
  bool x_75 = false;
  int x_29 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  int x_31 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  x_33 = vec4(0.0f);
  x_36 = x_29;
  x_38 = x_31;
  {
    while(true) {
      vec4 x_53 = vec4(0.0f);
      vec4 x_34 = vec4(0.0f);
      int x_62 = 0;
      int x_39 = 0;
      int x_41 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
      if ((x_38 < x_41)) {
      } else {
        break;
      }
      int x_56 = 0;
      switch(0u) {
        default:
        {
          if ((x_38 > v.tint_symbol_1.x_GLF_uniform_int_values[3].el)) {
            x_34 = x_33;
            x_62 = 2;
            break;
          }
          x_53 = x_33;
          x_56 = x_29;
          {
            while(true) {
              vec4 x_54 = vec4(0.0f);
              int x_57 = 0;
              if ((x_56 < x_41)) {
              } else {
                break;
              }
              {
                x_54 = vec4(float((x_38 + x_56)));
                x_57 = (x_56 + 1);
                x_53 = x_54;
                x_56 = x_57;
              }
              continue;
            }
          }
          x_GLF_color = x_53;
          x_34 = x_53;
          x_62 = x_31;
          break;
        }
      }
      {
        x_39 = (x_38 + 1);
        x_33 = x_34;
        x_36 = (x_36 + x_62);
        x_38 = x_39;
      }
      continue;
    }
  }
  vec4 v_1 = x_GLF_color;
  bool x_69 = all((v_1 == vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el))));
  x_75 = x_69;
  if (x_69) {
    x_74 = (x_36 == v.tint_symbol_1.x_GLF_uniform_int_values[5].el);
    x_75 = x_74;
  }
  if (x_75) {
    float x_79 = float(x_31);
    float x_80 = float(x_29);
    x_GLF_color = vec4(x_79, x_80, x_80, x_79);
  } else {
    x_GLF_color = vec4(float(x_29));
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:90: 'all' : no matching overloaded function found 
ERROR: 0:90: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:90: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
