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
  int x_59 = x_6.x_GLF_uniform_int_values[3].el;
  int x_61 = x_6.x_GLF_uniform_int_values[2].el;
  int x_63 = x_6.x_GLF_uniform_int_values[4].el;
  int x_65 = x_6.x_GLF_uniform_int_values[5].el;
  int x_67 = x_6.x_GLF_uniform_int_values[6].el;
  int x_69 = x_6.x_GLF_uniform_int_values[7].el;
  int x_71 = x_6.x_GLF_uniform_int_values[8].el;
  int x_73 = x_6.x_GLF_uniform_int_values[9].el;
  int x_75 = x_6.x_GLF_uniform_int_values[0].el;
  int x_77 = x_6.x_GLF_uniform_int_values[10].el;
  arr0 = int[10](x_59, x_61, x_63, x_65, x_67, x_69, x_71, x_73, x_75, x_77);
  int x_80 = x_6.x_GLF_uniform_int_values[1].el;
  int x_82 = x_6.x_GLF_uniform_int_values[12].el;
  int x_84 = x_6.x_GLF_uniform_int_values[15].el;
  int x_86 = x_6.x_GLF_uniform_int_values[16].el;
  int x_88 = x_6.x_GLF_uniform_int_values[17].el;
  int x_90 = x_6.x_GLF_uniform_int_values[13].el;
  int x_92 = x_6.x_GLF_uniform_int_values[14].el;
  int x_94 = x_6.x_GLF_uniform_int_values[11].el;
  int x_96 = x_6.x_GLF_uniform_int_values[18].el;
  int x_98 = x_6.x_GLF_uniform_int_values[19].el;
  arr1 = int[10](x_80, x_82, x_84, x_86, x_88, x_90, x_92, x_94, x_96, x_98);
  int x_101 = x_6.x_GLF_uniform_int_values[8].el;
  a = x_101;
  {
    while(true) {
      int x_106 = a;
      int x_108 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_106 < x_108)) {
      } else {
        break;
      }
      int x_112 = x_6.x_GLF_uniform_int_values[3].el;
      limiter0 = x_112;
      {
        while(true) {
          int x_117 = limiter0;
          int x_119 = x_6.x_GLF_uniform_int_values[4].el;
          if ((x_117 < x_119)) {
          } else {
            break;
          }
          int x_122 = limiter0;
          limiter0 = (x_122 + 1);
          int x_125 = x_6.x_GLF_uniform_int_values[2].el;
          limiter1 = x_125;
          int x_127 = x_6.x_GLF_uniform_int_values[3].el;
          b = x_127;
          {
            while(true) {
              int x_132 = b;
              int x_134 = x_6.x_GLF_uniform_int_values[1].el;
              if ((x_132 < x_134)) {
              } else {
                break;
              }
              int x_137 = limiter1;
              int x_139 = x_6.x_GLF_uniform_int_values[5].el;
              if ((x_137 > x_139)) {
                break;
              }
              int x_143 = limiter1;
              limiter1 = (x_143 + 1);
              int x_145 = b;
              int x_146 = a;
              int x_148 = arr1[x_146];
              arr0[x_145] = x_148;
              {
                int x_150 = b;
                b = (x_150 + 1);
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
          int x_156 = limiter2;
          if ((x_156 < 5)) {
          } else {
            break;
          }
          int x_159 = limiter2;
          limiter2 = (x_159 + 1);
          int x_162 = arr1[1];
          arr0[1] = x_162;
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
              int x_172 = d;
              if ((x_172 < 10)) {
              } else {
                break;
              }
              int x_175 = limiter3;
              if ((x_175 > 4)) {
                break;
              }
              int x_179 = limiter3;
              limiter3 = (x_179 + 1);
              int x_181 = d;
              int x_182 = d;
              int x_184 = arr0[x_182];
              arr1[x_181] = x_184;
              {
                int x_186 = d;
                d = (x_186 + 1);
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
        int x_193 = a;
        a = (x_193 + 1);
      }
      continue;
    }
  }
  int x_196 = x_6.x_GLF_uniform_int_values[11].el;
  int x_198 = x_6.x_GLF_uniform_int_values[12].el;
  int x_200 = x_6.x_GLF_uniform_int_values[11].el;
  int x_202 = x_6.x_GLF_uniform_int_values[5].el;
  int x_204 = x_6.x_GLF_uniform_int_values[6].el;
  int x_206 = x_6.x_GLF_uniform_int_values[7].el;
  int x_208 = x_6.x_GLF_uniform_int_values[8].el;
  int x_210 = x_6.x_GLF_uniform_int_values[9].el;
  int x_212 = x_6.x_GLF_uniform_int_values[0].el;
  int x_214 = x_6.x_GLF_uniform_int_values[10].el;
  ref0 = int[10](x_196, x_198, x_200, x_202, x_204, x_206, x_208, x_210, x_212, x_214);
  int x_217 = x_6.x_GLF_uniform_int_values[11].el;
  int x_219 = x_6.x_GLF_uniform_int_values[12].el;
  int x_221 = x_6.x_GLF_uniform_int_values[11].el;
  int x_223 = x_6.x_GLF_uniform_int_values[5].el;
  int x_225 = x_6.x_GLF_uniform_int_values[6].el;
  int x_227 = x_6.x_GLF_uniform_int_values[13].el;
  int x_229 = x_6.x_GLF_uniform_int_values[14].el;
  int x_231 = x_6.x_GLF_uniform_int_values[11].el;
  int x_233 = x_6.x_GLF_uniform_int_values[18].el;
  int x_235 = x_6.x_GLF_uniform_int_values[19].el;
  ref1 = int[10](x_217, x_219, x_221, x_223, x_225, x_227, x_229, x_231, x_233, x_235);
  int x_238 = x_6.x_GLF_uniform_int_values[2].el;
  int x_241 = x_6.x_GLF_uniform_int_values[3].el;
  int x_244 = x_6.x_GLF_uniform_int_values[3].el;
  int x_247 = x_6.x_GLF_uniform_int_values[2].el;
  float v = float(x_238);
  float v_1 = float(x_241);
  float v_2 = float(x_244);
  x_GLF_color = vec4(v, v_1, v_2, float(x_247));
  int x_251 = x_6.x_GLF_uniform_int_values[3].el;
  i = x_251;
  {
    while(true) {
      bool x_277 = false;
      bool x_278_phi = false;
      int x_256 = i;
      int x_258 = x_6.x_GLF_uniform_int_values[1].el;
      if ((x_256 < x_258)) {
      } else {
        break;
      }
      int x_261 = i;
      int x_263 = arr0[x_261];
      int x_264 = i;
      int x_266 = ref0[x_264];
      bool x_267 = (x_263 != x_266);
      x_278_phi = x_267;
      if (!(x_267)) {
        int x_271 = i;
        int x_273 = arr1[x_271];
        int x_274 = i;
        int x_276 = ref1[x_274];
        x_277 = (x_273 != x_276);
        x_278_phi = x_277;
      }
      bool x_278 = x_278_phi;
      if (x_278) {
        int x_282 = x_6.x_GLF_uniform_int_values[3].el;
        float x_283 = float(x_282);
        x_GLF_color = vec4(x_283, x_283, x_283, x_283);
      }
      {
        int x_285 = i;
        i = (x_285 + 1);
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
