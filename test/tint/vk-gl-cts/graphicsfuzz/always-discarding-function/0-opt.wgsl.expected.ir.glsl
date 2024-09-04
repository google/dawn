SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


struct tmp_struct {
  int nmb[1];
};

uniform buf0 x_11;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  int x_24[1] = int[1](0);
  bool x_68 = false;
  int x_17 = 0;
  int x_18 = 0;
  int x_19 = 0;
  int x_20 = 0;
  bool x_69 = false;
  float tmp_float = 0.0f;
  vec3 color = vec3(0.0f);
  {
    while(true) {
      int x_25 = 0;
      vec3 x_101 = vec3(0.0f);
      bool x_79_phi = false;
      int x_26_phi = 0;
      float x_75 = x_11.injectionSwitch.y;
      tmp_float = x_75;
      vec3 x_76 = vec3(x_75, x_75, x_75);
      color = x_76;
      x_24 = int[1](0);
      x_68 = false;
      x_79_phi = false;
      {
        while(true) {
          int x_21_phi = 0;
          int x_25_phi = 0;
          bool x_93_phi = false;
          bool x_79 = x_79_phi;
          x_18 = 1;
          x_21_phi = 1;
          {
            while(true) {
              int x_21 = x_21_phi;
              x_25_phi = 0;
              x_93_phi = x_79;
              if ((x_21 > 10)) {
              } else {
                break;
              }
              int x_22 = (x_21 - 1);
              x_19 = x_22;
              int x_23 = x_24[x_22];
              if ((x_23 == 1)) {
                x_68 = true;
                x_17 = 1;
                x_25_phi = 1;
                x_93_phi = true;
                break;
              }
              x_18 = x_22;
              {
                x_21_phi = x_22;
              }
              continue;
            }
          }
          x_25 = x_25_phi;
          bool x_93 = x_93_phi;
          x_26_phi = x_25;
          if (x_93) {
            break;
          }
          x_68 = true;
          x_17 = -1;
          x_26_phi = -1;
          break;
        }
      }
      int x_26 = x_26_phi;
      x_20 = x_26;
      if ((x_26 == -1)) {
        continue_execution = false;
      } else {
        x_GLF_color = vec4(0.0f);
        vec2 x_100 = (vec2(x_76[1u], x_76[2u]) + vec2(1.0f));
        x_101 = vec3(x_76[0u], x_100[0u], x_100[1u]);
        color = x_101;
        float x_103 = x_11.injectionSwitch.x;
        if ((x_103 > 1.0f)) {
          x_69 = true;
          break;
        }
      }
      x_GLF_color = vec4(x_101.x, x_101.y, x_101.z, 1.0f);
      x_69 = true;
      break;
    }
  }
}
main_out main() {
  main_1();
  main_out v = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v;
}
int binarySearch_struct_tmp_struct_i1_1_1_(inout tmp_struct obj) {
  bool x_112 = false;
  int x_16 = 0;
  int one = 0;
  int zero = 0;
  int x_27 = 0;
  bool x_114_phi = false;
  int x_28_phi = 0;
  x_114_phi = false;
  {
    while(true) {
      int x_15_phi = 0;
      int x_27_phi = 0;
      bool x_128_phi = false;
      bool x_114 = x_114_phi;
      one = 1;
      x_15_phi = 1;
      {
        while(true) {
          int x_15 = x_15_phi;
          x_27_phi = 0;
          x_128_phi = x_114;
          if ((x_15 > 10)) {
          } else {
            break;
          }
          int x_13 = (x_15 - 1);
          zero = x_13;
          int x_14 = obj.nmb[x_13];
          if ((x_14 == 1)) {
            x_112 = true;
            x_16 = 1;
            x_27_phi = 1;
            x_128_phi = true;
            break;
          }
          one = x_13;
          {
            x_15_phi = x_13;
          }
          continue;
        }
      }
      x_27 = x_27_phi;
      bool x_128 = x_128_phi;
      x_28_phi = x_27;
      if (x_128) {
        break;
      }
      x_112 = true;
      x_16 = -1;
      x_28_phi = -1;
      break;
    }
  }
  int x_28 = x_28_phi;
  return x_28;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
