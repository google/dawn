SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
vec3 f_() {
  int iteration = 0;
  int k = 0;
  iteration = 0;
  k = 0;
  {
    while(true) {
      if ((k < 100)) {
      } else {
        break;
      }
      iteration = (iteration + 1);
      {
        k = (k + 1);
      }
      continue;
    }
  }
  if ((iteration < 100)) {
    int x_13 = iteration;
    int x_15 = iteration;
    float v = float((x_13 - 1));
    return vec3(1.0f, v, float((x_15 - 1)));
  } else {
    {
      while(true) {
        {
          while(true) {
            return vec3(1.0f, 0.0f, 0.0f);
          }
        }
        /* unreachable */
      }
    }
  }
  /* unreachable */
}
void main_1() {
  vec3 x_35 = f_();
  x_GLF_color = vec4(x_35[0u], x_35[1u], x_35[2u], 1.0f);
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
