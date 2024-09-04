SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
vec3 func_() {
  vec2 v = vec2(0.0f);
  int i = 0;
  int k = 0;
  v = vec2(1.0f);
  i = 0;
  k = 0;
  {
    while(true) {
      int x_90 = k;
      if ((x_90 < 2)) {
      } else {
        break;
      }
      float x_94 = v.y;
      if (((x_94 + 1.0f) > 4.0f)) {
        break;
      }
      v[1u] = 1.0f;
      int x_100 = i;
      i = (x_100 + 1);
      {
        int x_102 = k;
        k = (x_102 + 1);
      }
      continue;
    }
  }
  int x_104 = i;
  if ((x_104 < 10)) {
    return vec3(1.0f, 0.0f, 0.0f);
  } else {
    return vec3(0.0f, 0.0f, 1.0f);
  }
  /* unreachable */
}
void main_1() {
  int j = 0;
  vec3 data[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  int j_1 = 0;
  bool x_80 = false;
  bool x_81_phi = false;
  j = 0;
  {
    while(true) {
      int x_49 = j;
      if ((x_49 < 1)) {
      } else {
        break;
      }
      int x_52 = j;
      vec3 x_53 = func_();
      data[x_52] = x_53;
      {
        int x_55 = j;
        j = (x_55 + 1);
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_61 = j_1;
      if ((x_61 < 1)) {
      } else {
        break;
      }
      int x_64 = j_1;
      vec3 x_67 = func_();
      data[((4 * x_64) + 1)] = x_67;
      {
        int x_69 = j_1;
        j_1 = (x_69 + 1);
      }
      continue;
    }
  }
  vec3 x_72 = data[0];
  bool x_74 = all((x_72 == vec3(1.0f, 0.0f, 0.0f)));
  x_81_phi = x_74;
  if (x_74) {
    vec3 x_78 = data[1];
    x_80 = all((x_78 == vec3(1.0f, 0.0f, 0.0f)));
    x_81_phi = x_80;
  }
  bool x_81 = x_81_phi;
  if (x_81) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
