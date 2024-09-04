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
      if ((k < 2)) {
      } else {
        break;
      }
      if (((v.y + 1.0f) > 4.0f)) {
        break;
      }
      v[1u] = 1.0f;
      i = (i + 1);
      {
        k = (k + 1);
      }
      continue;
    }
  }
  if ((i < 10)) {
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
  bool x_81 = false;
  j = 0;
  {
    while(true) {
      if ((j < 1)) {
      } else {
        break;
      }
      int x_52 = j;
      vec3 x_53 = func_();
      data[x_52] = x_53;
      {
        j = (j + 1);
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      if ((j_1 < 1)) {
      } else {
        break;
      }
      int x_64 = j_1;
      vec3 x_67 = func_();
      data[((4 * x_64) + 1)] = x_67;
      {
        j_1 = (j_1 + 1);
      }
      continue;
    }
  }
  bool x_74 = all((data[0] == vec3(1.0f, 0.0f, 0.0f)));
  x_81 = x_74;
  if (x_74) {
    x_80 = all((data[1] == vec3(1.0f, 0.0f, 0.0f)));
    x_81 = x_80;
  }
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
