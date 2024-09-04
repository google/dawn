SKIP: FAILED

#version 310 es

struct buf0 {
  mat4 matrix_a_uni;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int x = 0;
  vec4 matrix_u = vec4(0.0f);
  int b = 0;
  vec4 matrix_b = vec4(0.0f);
  vec4 x_42 = vec4(0.0f);
  x = 4;
  {
    while(true) {
      if ((x >= 1)) {
      } else {
        break;
      }
      int x_11 = x;
      matrix_u[x_11] = 1.0f;
      {
        x = (x - 1);
      }
      continue;
    }
  }
  b = 4;
  {
    while(true) {
      if ((x_8.matrix_a_uni[0].x < -1.0f)) {
      } else {
        break;
      }
      int x_14 = b;
      if ((b > 1)) {
        x_42 = min(matrix_b, matrix_b);
      } else {
        x_42 = matrix_u;
      }
      matrix_b[x_14] = x_42.y;
      {
        b = (b - 1);
      }
      continue;
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
