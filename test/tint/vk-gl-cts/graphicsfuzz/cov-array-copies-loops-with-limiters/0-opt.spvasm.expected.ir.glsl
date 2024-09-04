SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[20];
};

struct buf1 {
  int one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_19;
void main_1() {
  int arr0[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int arr1[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int a = 0;
  int limiter0 = 0;
  int limiter1 = 0;
  int b = 0;
  int limiter2 = 0;
  int limiter3 = 0;
  int d = 0;
  int ref0[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int ref1[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i = 0;
  arr0 = int[10](x_6.x_GLF_uniform_int_values[3].el, x_6.x_GLF_uniform_int_values[2].el, x_6.x_GLF_uniform_int_values[4].el, x_6.x_GLF_uniform_int_values[5].el, x_6.x_GLF_uniform_int_values[6].el, x_6.x_GLF_uniform_int_values[7].el, x_6.x_GLF_uniform_int_values[8].el, x_6.x_GLF_uniform_int_values[9].el, x_6.x_GLF_uniform_int_values[0].el, x_6.x_GLF_uniform_int_values[10].el);
  arr1 = int[10](x_6.x_GLF_uniform_int_values[1].el, x_6.x_GLF_uniform_int_values[12].el, x_6.x_GLF_uniform_int_values[15].el, x_6.x_GLF_uniform_int_values[16].el, x_6.x_GLF_uniform_int_values[17].el, x_6.x_GLF_uniform_int_values[13].el, x_6.x_GLF_uniform_int_values[14].el, x_6.x_GLF_uniform_int_values[11].el, x_6.x_GLF_uniform_int_values[18].el, x_6.x_GLF_uniform_int_values[19].el);
  a = x_6.x_GLF_uniform_int_values[8].el;
  {
    while(true) {
      if ((a < x_6.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      limiter0 = x_6.x_GLF_uniform_int_values[3].el;
      {
        while(true) {
          if ((limiter0 < x_6.x_GLF_uniform_int_values[4].el)) {
          } else {
            break;
          }
          limiter0 = (limiter0 + 1);
          limiter1 = x_6.x_GLF_uniform_int_values[2].el;
          b = x_6.x_GLF_uniform_int_values[3].el;
          {
            while(true) {
              if ((b < x_6.x_GLF_uniform_int_values[1].el)) {
              } else {
                break;
              }
              if ((limiter1 > x_6.x_GLF_uniform_int_values[5].el)) {
                break;
              }
              limiter1 = (limiter1 + 1);
              int x_145 = b;
              arr0[x_145] = arr1[a];
              {
                b = (b + 1);
              }
              continue;
            }
          }
          {
          }
          continue;
        }
      }
      limiter2 = 0;
      {
        while(true) {
          if ((limiter2 < 5)) {
          } else {
            break;
          }
          limiter2 = (limiter2 + 1);
          arr0[1] = arr1[1];
          {
          }
          continue;
        }
      }
      {
        while(true) {
          limiter3 = 0;
          d = 0;
          {
            while(true) {
              if ((d < 10)) {
              } else {
                break;
              }
              if ((limiter3 > 4)) {
                break;
              }
              limiter3 = (limiter3 + 1);
              int x_181 = d;
              arr1[x_181] = arr0[d];
              {
                d = (d + 1);
              }
              continue;
            }
          }
          {
            int x_189 = x_6.x_GLF_uniform_int_values[2].el;
            int x_191 = x_6.x_GLF_uniform_int_values[3].el;
            if (!((x_189 == x_191))) { break; }
          }
          continue;
        }
      }
      {
        a = (a + 1);
      }
      continue;
    }
  }
  ref0 = int[10](x_6.x_GLF_uniform_int_values[11].el, x_6.x_GLF_uniform_int_values[12].el, x_6.x_GLF_uniform_int_values[11].el, x_6.x_GLF_uniform_int_values[5].el, x_6.x_GLF_uniform_int_values[6].el, x_6.x_GLF_uniform_int_values[7].el, x_6.x_GLF_uniform_int_values[8].el, x_6.x_GLF_uniform_int_values[9].el, x_6.x_GLF_uniform_int_values[0].el, x_6.x_GLF_uniform_int_values[10].el);
  ref1 = int[10](x_6.x_GLF_uniform_int_values[11].el, x_6.x_GLF_uniform_int_values[12].el, x_6.x_GLF_uniform_int_values[11].el, x_6.x_GLF_uniform_int_values[5].el, x_6.x_GLF_uniform_int_values[6].el, x_6.x_GLF_uniform_int_values[13].el, x_6.x_GLF_uniform_int_values[14].el, x_6.x_GLF_uniform_int_values[11].el, x_6.x_GLF_uniform_int_values[18].el, x_6.x_GLF_uniform_int_values[19].el);
  float v = float(x_6.x_GLF_uniform_int_values[2].el);
  float v_1 = float(x_6.x_GLF_uniform_int_values[3].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[3].el);
  x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[2].el));
  i = x_6.x_GLF_uniform_int_values[3].el;
  {
    while(true) {
      bool x_277 = false;
      bool x_278 = false;
      if ((i < x_6.x_GLF_uniform_int_values[1].el)) {
      } else {
        break;
      }
      bool x_267 = (arr0[i] != ref0[i]);
      x_278 = x_267;
      if (!(x_267)) {
        x_277 = (arr1[i] != ref1[i]);
        x_278 = x_277;
      }
      if (x_278) {
        x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[3].el));
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
