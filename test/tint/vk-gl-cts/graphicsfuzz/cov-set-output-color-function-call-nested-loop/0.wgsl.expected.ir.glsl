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
  int x_36 = 0;
  bool x_74 = false;
  vec4 x_33_phi = vec4(0.0f);
  int x_36_phi = 0;
  int x_38_phi = 0;
  bool x_75_phi = false;
  int x_29 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  int x_31 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  x_33_phi = vec4(0.0f);
  x_36_phi = x_29;
  x_38_phi = x_31;
  {
    while(true) {
      vec4 x_53 = vec4(0.0f);
      int x_39 = 0;
      vec4 x_34_phi = vec4(0.0f);
      int x_62_phi = 0;
      vec4 x_33 = x_33_phi;
      x_36 = x_36_phi;
      int x_38 = x_38_phi;
      int x_41 = v.tint_symbol_1.x_GLF_uniform_int_values[4].el;
      if ((x_38 < x_41)) {
      } else {
        break;
      }
      vec4 x_53_phi = vec4(0.0f);
      int x_56_phi = 0;
      switch(0u) {
        default:
        {
          int x_48 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
          if ((x_38 > x_48)) {
            x_34_phi = x_33;
            x_62_phi = 2;
            break;
          }
          x_53_phi = x_33;
          x_56_phi = x_29;
          {
            while(true) {
              vec4 x_54 = vec4(0.0f);
              int x_57 = 0;
              x_53 = x_53_phi;
              int x_56 = x_56_phi;
              if ((x_56 < x_41)) {
              } else {
                break;
              }
              {
                float x_61 = float((x_38 + x_56));
                x_54 = vec4(x_61, x_61, x_61, x_61);
                x_57 = (x_56 + 1);
                x_53_phi = x_54;
                x_56_phi = x_57;
              }
              continue;
            }
          }
          x_GLF_color = x_53;
          x_34_phi = x_53;
          x_62_phi = x_31;
          break;
        }
      }
      vec4 x_34 = x_34_phi;
      int x_62 = x_62_phi;
      {
        x_39 = (x_38 + 1);
        x_33_phi = x_34;
        x_36_phi = (x_36 + x_62);
        x_38_phi = x_39;
      }
      continue;
    }
  }
  vec4 x_63 = x_GLF_color;
  int x_65 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  float x_66 = float(x_65);
  bool x_69 = all((x_63 == vec4(x_66, x_66, x_66, x_66)));
  x_75_phi = x_69;
  if (x_69) {
    int x_73 = v.tint_symbol_1.x_GLF_uniform_int_values[5].el;
    x_74 = (x_36 == x_73);
    x_75_phi = x_74;
  }
  bool x_75 = x_75_phi;
  if (x_75) {
    float x_79 = float(x_31);
    float x_80 = float(x_29);
    x_GLF_color = vec4(x_79, x_80, x_80, x_79);
  } else {
    float x_82 = float(x_29);
    x_GLF_color = vec4(x_82, x_82, x_82, x_82);
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
ERROR: 0:103: 'all' : no matching overloaded function found 
ERROR: 0:103: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:103: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
