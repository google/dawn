SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[7];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v1 = vec4(0.0f);
  vec4 v2 = vec4(0.0f);
  vec4 v3 = vec4(0.0f);
  vec4 v4 = vec4(0.0f);
  bool x_69 = false;
  bool x_77 = false;
  bool x_85 = false;
  bool x_93 = false;
  bool x_70_phi = false;
  bool x_78_phi = false;
  bool x_86_phi = false;
  bool x_94_phi = false;
  float x_41 = x_6.x_GLF_uniform_float_values[2].el;
  float x_43 = x_6.x_GLF_uniform_float_values[2].el;
  float x_45 = x_6.x_GLF_uniform_float_values[0].el;
  float x_47 = x_6.x_GLF_uniform_float_values[2].el;
  v1 = vec4(x_41, x_43, x_45, x_47);
  v2 = vec4(1.57079637050628662109f, 1.11976957321166992188f, 1.0f, 0.92729520797729492188f);
  float x_50 = x_6.x_GLF_uniform_float_values[0].el;
  v3 = vec4(x_50, x_50, x_50, x_50);
  vec4 x_52 = v1;
  vec4 x_53 = v2;
  vec4 x_54 = v3;
  v4 = smoothstep(x_52, x_53, x_54);
  vec4 x_56 = v4;
  x_GLF_color = vec4(x_56[0u], x_56[1u], x_56[3u], x_56[0u]);
  float x_59 = v4.x;
  float x_61 = x_6.x_GLF_uniform_float_values[4].el;
  bool x_62 = (x_59 > x_61);
  x_70_phi = x_62;
  if (x_62) {
    float x_66 = v4.x;
    float x_68 = x_6.x_GLF_uniform_float_values[5].el;
    x_69 = (x_66 < x_68);
    x_70_phi = x_69;
  }
  bool x_70 = x_70_phi;
  x_78_phi = x_70;
  if (x_70) {
    float x_74 = v4.y;
    float x_76 = x_6.x_GLF_uniform_float_values[3].el;
    x_77 = (x_74 > x_76);
    x_78_phi = x_77;
  }
  bool x_78 = x_78_phi;
  x_86_phi = x_78;
  if (x_78) {
    float x_82 = v4.y;
    float x_84 = x_6.x_GLF_uniform_float_values[6].el;
    x_85 = (x_82 < x_84);
    x_86_phi = x_85;
  }
  bool x_86 = x_86_phi;
  x_94_phi = x_86;
  if (x_86) {
    float x_90 = v4.w;
    float x_92 = x_6.x_GLF_uniform_float_values[0].el;
    x_93 = (x_90 == x_92);
    x_94_phi = x_93;
  }
  bool x_94 = x_94_phi;
  if (x_94) {
    float x_99 = x_6.x_GLF_uniform_float_values[0].el;
    float x_101 = x_6.x_GLF_uniform_float_values[1].el;
    float x_103 = x_6.x_GLF_uniform_float_values[1].el;
    float x_105 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_99, x_101, x_103, x_105);
  } else {
    float x_108 = x_6.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4(x_108, x_108, x_108, x_108);
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
