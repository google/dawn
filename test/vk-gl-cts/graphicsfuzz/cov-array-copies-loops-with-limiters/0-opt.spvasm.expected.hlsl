cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[20];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_19 : register(b1, space0) {
  uint4 x_19[1];
};

void main_1() {
  int arr0[10] = (int[10])0;
  int arr1[10] = (int[10])0;
  int a = 0;
  int limiter0 = 0;
  int limiter1 = 0;
  int b = 0;
  int limiter2 = 0;
  int limiter3 = 0;
  int d = 0;
  int ref0[10] = (int[10])0;
  int ref1[10] = (int[10])0;
  int i = 0;
  const int x_59 = asint(x_6[3].x);
  const int x_61 = asint(x_6[2].x);
  const int x_63 = asint(x_6[4].x);
  const int x_65 = asint(x_6[5].x);
  const int x_67 = asint(x_6[6].x);
  const int x_69 = asint(x_6[7].x);
  const int x_71 = asint(x_6[8].x);
  const int x_73 = asint(x_6[9].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_75 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const int x_77 = asint(x_6[10].x);
  const int tint_symbol_2[10] = {x_59, x_61, x_63, x_65, x_67, x_69, x_71, x_73, x_75, x_77};
  arr0 = tint_symbol_2;
  const int x_80 = asint(x_6[1].x);
  const int x_82 = asint(x_6[12].x);
  const int x_84 = asint(x_6[15].x);
  const int x_86 = asint(x_6[16].x);
  const int x_88 = asint(x_6[17].x);
  const int x_90 = asint(x_6[13].x);
  const int x_92 = asint(x_6[14].x);
  const int x_94 = asint(x_6[11].x);
  const int x_96 = asint(x_6[18].x);
  const int x_98 = asint(x_6[19].x);
  const int tint_symbol_3[10] = {x_80, x_82, x_84, x_86, x_88, x_90, x_92, x_94, x_96, x_98};
  arr1 = tint_symbol_3;
  const int x_101 = asint(x_6[8].x);
  a = x_101;
  while (true) {
    const int x_106 = a;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_108 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_106 < x_108)) {
    } else {
      break;
    }
    const int x_112 = asint(x_6[3].x);
    limiter0 = x_112;
    while (true) {
      const int x_117 = limiter0;
      const int x_119 = asint(x_6[4].x);
      if ((x_117 < x_119)) {
      } else {
        break;
      }
      limiter0 = (limiter0 + 1);
      const int x_125 = asint(x_6[2].x);
      limiter1 = x_125;
      const int x_127 = asint(x_6[3].x);
      b = x_127;
      while (true) {
        const int x_132 = b;
        const int x_134 = asint(x_6[1].x);
        if ((x_132 < x_134)) {
        } else {
          break;
        }
        const int x_137 = limiter1;
        const int x_139 = asint(x_6[5].x);
        if ((x_137 > x_139)) {
          break;
        }
        limiter1 = (limiter1 + 1);
        const int x_145 = b;
        const int x_148 = arr1[a];
        arr0[x_145] = x_148;
        {
          b = (b + 1);
        }
      }
    }
    limiter2 = 0;
    while (true) {
      if ((limiter2 < 5)) {
      } else {
        break;
      }
      limiter2 = (limiter2 + 1);
      const int x_162 = arr1[1];
      arr0[1] = x_162;
    }
    while (true) {
      limiter3 = 0;
      d = 0;
      {
        for(; (d < 10); d = (d + 1)) {
          if ((limiter3 > 4)) {
            break;
          }
          limiter3 = (limiter3 + 1);
          const int x_181 = d;
          const int x_184 = arr0[d];
          arr1[x_181] = x_184;
        }
      }
      {
        const int x_189 = asint(x_6[2].x);
        const int x_191 = asint(x_6[3].x);
        if ((x_189 == x_191)) {
        } else {
          break;
        }
      }
    }
    {
      a = (a + 1);
    }
  }
  const int x_196 = asint(x_6[11].x);
  const int x_198 = asint(x_6[12].x);
  const int x_200 = asint(x_6[11].x);
  const int x_202 = asint(x_6[5].x);
  const int x_204 = asint(x_6[6].x);
  const int x_206 = asint(x_6[7].x);
  const int x_208 = asint(x_6[8].x);
  const int x_210 = asint(x_6[9].x);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_212 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const int x_214 = asint(x_6[10].x);
  const int tint_symbol_4[10] = {x_196, x_198, x_200, x_202, x_204, x_206, x_208, x_210, x_212, x_214};
  ref0 = tint_symbol_4;
  const int x_217 = asint(x_6[11].x);
  const int x_219 = asint(x_6[12].x);
  const int x_221 = asint(x_6[11].x);
  const int x_223 = asint(x_6[5].x);
  const int x_225 = asint(x_6[6].x);
  const int x_227 = asint(x_6[13].x);
  const int x_229 = asint(x_6[14].x);
  const int x_231 = asint(x_6[11].x);
  const int x_233 = asint(x_6[18].x);
  const int x_235 = asint(x_6[19].x);
  const int tint_symbol_5[10] = {x_217, x_219, x_221, x_223, x_225, x_227, x_229, x_231, x_233, x_235};
  ref1 = tint_symbol_5;
  const int x_238 = asint(x_6[2].x);
  const int x_241 = asint(x_6[3].x);
  const int x_244 = asint(x_6[3].x);
  const int x_247 = asint(x_6[2].x);
  x_GLF_color = float4(float(x_238), float(x_241), float(x_244), float(x_247));
  const int x_251 = asint(x_6[3].x);
  i = x_251;
  while (true) {
    bool x_277 = false;
    bool x_278_phi = false;
    const int x_256 = i;
    const int x_258 = asint(x_6[1].x);
    if ((x_256 < x_258)) {
    } else {
      break;
    }
    const int x_263 = arr0[i];
    const int x_266 = ref0[i];
    const bool x_267 = (x_263 != x_266);
    x_278_phi = x_267;
    if (!(x_267)) {
      const int x_273 = arr1[i];
      const int x_276 = ref1[i];
      x_277 = (x_273 != x_276);
      x_278_phi = x_277;
    }
    if (x_278_phi) {
      const int x_282 = asint(x_6[3].x);
      const float x_283 = float(x_282);
      x_GLF_color = float4(x_283, x_283, x_283, x_283);
    }
    {
      i = (i + 1);
    }
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
