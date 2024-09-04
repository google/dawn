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
      int x_79 = k;
      if ((x_79 < 2)) {
      } else {
        break;
      }
      float x_83 = v.y;
      if (((x_83 + 1.0f) > 4.0f)) {
        break;
      }
      v[1u] = 1.0f;
      int x_89 = i;
      i = (x_89 + 1);
      {
        int x_91 = k;
        k = (x_91 + 1);
      }
      continue;
    }
  }
  int x_93 = i;
  if ((x_93 < 10)) {
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
  j = 0;
  {
    while(true) {
      int x_46 = j;
      if ((x_46 < 1)) {
      } else {
        break;
      }
      int x_49 = j;
      vec3 x_50 = func_();
      data[x_49] = x_50;
      {
        int x_52 = j;
        j = (x_52 + 1);
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_58 = j_1;
      if ((x_58 < 1)) {
      } else {
        break;
      }
      int x_61 = j_1;
      vec3 x_64 = func_();
      data[((4 * x_61) + 1)] = x_64;
      {
        int x_66 = j_1;
        j_1 = (x_66 + 1);
      }
      continue;
    }
  }
  vec3 x_69 = data[0];
  x_GLF_color = vec4(x_69[0u], x_69[1u], x_69[2u], 1.0f);
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
