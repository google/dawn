static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static int map[256] = (int[256])0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 pos = float2(0.0f, 0.0f);
  int2 ipos = int2(0, 0);
  int i = 0;
  int2 p = int2(0, 0);
  bool canwalk = false;
  int v = 0;
  int directions = 0;
  int j = 0;
  int d = 0;
  const float4 x_57 = gl_FragCoord;
  const float2 x_60 = asfloat(x_7[0].xy);
  pos = (float2(x_57.x, x_57.y) / x_60);
  const float x_63 = pos.x;
  const float x_67 = pos.y;
  ipos = int2(int((x_63 * 16.0f)), int((x_67 * 16.0f)));
  i = 0;
  {
    for(; (i < 256); i = (i + 1)) {
      map[i] = 0;
    }
  }
  p = int2(0, 0);
  canwalk = true;
  v = 0;
  while (true) {
    bool x_102 = false;
    bool x_122 = false;
    bool x_142 = false;
    bool x_162 = false;
    bool x_103_phi = false;
    bool x_123_phi = false;
    bool x_143_phi = false;
    bool x_163_phi = false;
    v = (v + 1);
    directions = 0;
    const int x_89 = p.x;
    const bool x_90 = (x_89 > 0);
    x_103_phi = x_90;
    if (x_90) {
      const int x_94 = p.x;
      const int x_97 = p.y;
      const int x_101 = map[((x_94 - 2) + (x_97 * 16))];
      x_102 = (x_101 == 0);
      x_103_phi = x_102;
    }
    if (x_103_phi) {
      directions = (directions + 1);
    }
    const int x_109 = p.y;
    const bool x_110 = (x_109 > 0);
    x_123_phi = x_110;
    if (x_110) {
      const int x_114 = p.x;
      const int x_116 = p.y;
      const int x_121 = map[(x_114 + ((x_116 - 2) * 16))];
      x_122 = (x_121 == 0);
      x_123_phi = x_122;
    }
    if (x_123_phi) {
      directions = (directions + 1);
    }
    const int x_129 = p.x;
    const bool x_130 = (x_129 < 14);
    x_143_phi = x_130;
    if (x_130) {
      const int x_134 = p.x;
      const int x_137 = p.y;
      const int x_141 = map[((x_134 + 2) + (x_137 * 16))];
      x_142 = (x_141 == 0);
      x_143_phi = x_142;
    }
    if (x_143_phi) {
      directions = (directions + 1);
    }
    const int x_149 = p.y;
    const bool x_150 = (x_149 < 14);
    x_163_phi = x_150;
    if (x_150) {
      const int x_154 = p.x;
      const int x_156 = p.y;
      const int x_161 = map[(x_154 + ((x_156 + 2) * 16))];
      x_162 = (x_161 == 0);
      x_163_phi = x_162;
    }
    if (x_163_phi) {
      directions = (directions + 1);
    }
    bool x_227 = false;
    bool x_240 = false;
    bool x_279 = false;
    bool x_292 = false;
    bool x_331 = false;
    bool x_344 = false;
    bool x_383 = false;
    bool x_396 = false;
    bool x_228_phi = false;
    bool x_241_phi = false;
    bool x_280_phi = false;
    bool x_293_phi = false;
    bool x_332_phi = false;
    bool x_345_phi = false;
    bool x_384_phi = false;
    bool x_397_phi = false;
    if ((directions == 0)) {
      canwalk = false;
      i = 0;
      {
        for(; (i < 8); i = (i + 1)) {
          j = 0;
          {
            for(; (j < 8); j = (j + 1)) {
              const int x_194 = map[((j * 2) + ((i * 2) * 16))];
              if ((x_194 == 0)) {
                p.x = (j * 2);
                p.y = (i * 2);
                canwalk = true;
              }
            }
          }
        }
      }
      const int x_209 = p.x;
      const int x_211 = p.y;
      map[(x_209 + (x_211 * 16))] = 1;
    } else {
      d = (v % directions);
      v = (v + directions);
      const bool x_222 = (d >= 0);
      x_228_phi = x_222;
      if (x_222) {
        const int x_226 = p.x;
        x_227 = (x_226 > 0);
        x_228_phi = x_227;
      }
      const bool x_228 = x_228_phi;
      x_241_phi = x_228;
      if (x_228) {
        const int x_232 = p.x;
        const int x_235 = p.y;
        const int x_239 = map[((x_232 - 2) + (x_235 * 16))];
        x_240 = (x_239 == 0);
        x_241_phi = x_240;
      }
      if (x_241_phi) {
        d = (d - 1);
        const int x_247 = p.x;
        const int x_249 = p.y;
        map[(x_247 + (x_249 * 16))] = 1;
        const int x_254 = p.x;
        const int x_257 = p.y;
        map[((x_254 - 1) + (x_257 * 16))] = 1;
        const int x_262 = p.x;
        const int x_265 = p.y;
        map[((x_262 - 2) + (x_265 * 16))] = 1;
        const int x_270 = p.x;
        p.x = (x_270 - 2);
      }
      const bool x_274 = (d >= 0);
      x_280_phi = x_274;
      if (x_274) {
        const int x_278 = p.y;
        x_279 = (x_278 > 0);
        x_280_phi = x_279;
      }
      const bool x_280 = x_280_phi;
      x_293_phi = x_280;
      if (x_280) {
        const int x_284 = p.x;
        const int x_286 = p.y;
        const int x_291 = map[(x_284 + ((x_286 - 2) * 16))];
        x_292 = (x_291 == 0);
        x_293_phi = x_292;
      }
      if (x_293_phi) {
        d = (d - 1);
        const int x_299 = p.x;
        const int x_301 = p.y;
        map[(x_299 + (x_301 * 16))] = 1;
        const int x_306 = p.x;
        const int x_308 = p.y;
        map[(x_306 + ((x_308 - 1) * 16))] = 1;
        const int x_314 = p.x;
        const int x_316 = p.y;
        map[(x_314 + ((x_316 - 2) * 16))] = 1;
        const int x_322 = p.y;
        p.y = (x_322 - 2);
      }
      const bool x_326 = (d >= 0);
      x_332_phi = x_326;
      if (x_326) {
        const int x_330 = p.x;
        x_331 = (x_330 < 14);
        x_332_phi = x_331;
      }
      const bool x_332 = x_332_phi;
      x_345_phi = x_332;
      if (x_332) {
        const int x_336 = p.x;
        const int x_339 = p.y;
        const int x_343 = map[((x_336 + 2) + (x_339 * 16))];
        x_344 = (x_343 == 0);
        x_345_phi = x_344;
      }
      if (x_345_phi) {
        d = (d - 1);
        const int x_351 = p.x;
        const int x_353 = p.y;
        map[(x_351 + (x_353 * 16))] = 1;
        const int x_358 = p.x;
        const int x_361 = p.y;
        map[((x_358 + 1) + (x_361 * 16))] = 1;
        const int x_366 = p.x;
        const int x_369 = p.y;
        map[((x_366 + 2) + (x_369 * 16))] = 1;
        const int x_374 = p.x;
        p.x = (x_374 + 2);
      }
      const bool x_378 = (d >= 0);
      x_384_phi = x_378;
      if (x_378) {
        const int x_382 = p.y;
        x_383 = (x_382 < 14);
        x_384_phi = x_383;
      }
      const bool x_384 = x_384_phi;
      x_397_phi = x_384;
      if (x_384) {
        const int x_388 = p.x;
        const int x_390 = p.y;
        const int x_395 = map[(x_388 + ((x_390 + 2) * 16))];
        x_396 = (x_395 == 0);
        x_397_phi = x_396;
      }
      if (x_397_phi) {
        d = (d - 1);
        const int x_403 = p.x;
        const int x_405 = p.y;
        map[(x_403 + (x_405 * 16))] = 1;
        const int x_410 = p.x;
        const int x_412 = p.y;
        map[(x_410 + ((x_412 + 1) * 16))] = 1;
        const int x_418 = p.x;
        const int x_420 = p.y;
        map[(x_418 + ((x_420 + 2) * 16))] = 1;
        const int x_426 = p.y;
        p.y = (x_426 + 2);
      }
    }
    const int x_430 = ipos.y;
    const int x_433 = ipos.x;
    const int x_436 = map[((x_430 * 16) + x_433)];
    if ((x_436 == 1)) {
      x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
      return;
    }
    {
      if (canwalk) {
      } else {
        break;
      }
    }
  }
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
