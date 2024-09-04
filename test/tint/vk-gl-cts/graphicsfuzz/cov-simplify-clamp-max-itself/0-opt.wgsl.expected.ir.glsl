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
      int x_40 = i;
      int x_42 = x_7.sequence.w;
      if ((x_40 < (x_42 + 1))) {
      } else {
        break;
      }
      int x_46 = i;
      int x_48 = x_7.sequence.x;
      int x_49 = i;
      int x_52 = x_7.sequence[min(max(x_46, x_48), x_49)];
      if ((x_52 == 1)) {
        int x_57 = i;
        a[x_57] = 5;
      } else {
        int x_59 = i;
        int x_60 = i;
        a[x_59] = x_60;
      }
      {
        int x_62 = i;
        i = (x_62 + 1);
      }
      continue;
    }
  }
  int x_65 = a.x;
  int x_67 = a.y;
  int x_70 = a.z;
  int x_73 = a.w;
  sum = (((x_65 + x_67) + x_70) + x_73);
  int x_75 = sum;
  if ((x_75 == 10)) {
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
