SKIP: FAILED

#version 310 es

struct buf0 {
  ivec4 sequence;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  ivec4 a = ivec4(0);
  int i = 0;
  int sum = 0;
  a = ivec4(0);
  i = 0;
  {
    while(true) {
      if ((i < (x_7.sequence.w + 1))) {
      } else {
        break;
      }
      int v = i;
      if ((x_7.sequence[min(max(i, x_7.sequence.x), v)] == 1)) {
        int x_57 = i;
        a[x_57] = 5;
      } else {
        int x_59 = i;
        a[x_59] = i;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  sum = (((a.x + a.y) + a.z) + a.w);
  if ((sum == 10)) {
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
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
