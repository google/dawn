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
  const float4 x_59 = gl_FragCoord;
  const float2 x_62 = asfloat(x_7[0].xy);
  pos = (float2(x_59.x, x_59.y) / x_62);
  const float x_65 = pos.x;
  const float x_69 = pos.y;
  ipos = int2(int((x_65 * 16.0f)), int((x_69 * 16.0f)));
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
    bool x_104 = false;
    bool x_124 = false;
    bool x_144 = false;
    bool x_164 = false;
    bool x_105_phi = false;
    bool x_125_phi = false;
    bool x_145_phi = false;
    bool x_165_phi = false;
    v = (v + 1);
    directions = 0;
    const int x_91 = p.x;
    const bool x_92 = (x_91 > 0);
    x_105_phi = x_92;
    if (x_92) {
      const int x_96 = p.x;
      const int x_99 = p.y;
      const int x_103 = map[((x_96 - 2) + (x_99 * 16))];
      x_104 = (x_103 == 0);
      x_105_phi = x_104;
    }
    if (x_105_phi) {
      directions = (directions + 1);
    }
    const int x_111 = p.y;
    const bool x_112 = (x_111 > 0);
    x_125_phi = x_112;
    if (x_112) {
      const int x_116 = p.x;
      const int x_118 = p.y;
      const int x_123 = map[(x_116 + ((x_118 - 2) * 16))];
      x_124 = (x_123 == 0);
      x_125_phi = x_124;
    }
    if (x_125_phi) {
      directions = (directions + 1);
    }
    const int x_131 = p.x;
    const bool x_132 = (x_131 < 14);
    x_145_phi = x_132;
    if (x_132) {
      const int x_136 = p.x;
      const int x_139 = p.y;
      const int x_143 = map[((x_136 + 2) + (x_139 * 16))];
      x_144 = (x_143 == 0);
      x_145_phi = x_144;
    }
    if (x_145_phi) {
      directions = (directions + 1);
    }
    const int x_151 = p.y;
    const bool x_152 = (x_151 < 14);
    x_165_phi = x_152;
    if (x_152) {
      const int x_156 = p.x;
      const int x_158 = p.y;
      const int x_163 = map[(x_156 + ((x_158 + 2) * 16))];
      x_164 = (x_163 == 0);
      x_165_phi = x_164;
    }
    if (x_165_phi) {
      directions = (directions + 1);
    }
    bool x_229 = false;
    bool x_242 = false;
    bool x_281 = false;
    bool x_295 = false;
    bool x_335 = false;
    bool x_348 = false;
    bool x_387 = false;
    bool x_400 = false;
    bool x_230_phi = false;
    bool x_243_phi = false;
    bool x_282_phi = false;
    bool x_296_phi = false;
    bool x_336_phi = false;
    bool x_349_phi = false;
    bool x_388_phi = false;
    bool x_401_phi = false;
    if ((directions == 0)) {
      canwalk = false;
      i = 0;
      {
        for(; (i < 8); i = (i + 1)) {
          j = 0;
          {
            for(; (j < 8); j = (j + 1)) {
              const int x_196 = map[((j * 2) + ((i * 2) * 16))];
              if ((x_196 == 0)) {
                p.x = (j * 2);
                p.y = (i * 2);
                canwalk = true;
              }
            }
          }
        }
      }
      const int x_211 = p.x;
      const int x_213 = p.y;
      map[(x_211 + (x_213 * 16))] = 1;
    } else {
      d = (v % directions);
      v = (v + directions);
      const bool x_224 = (d >= 0);
      x_230_phi = x_224;
      if (x_224) {
        const int x_228 = p.x;
        x_229 = (x_228 > 0);
        x_230_phi = x_229;
      }
      const bool x_230 = x_230_phi;
      x_243_phi = x_230;
      if (x_230) {
        const int x_234 = p.x;
        const int x_237 = p.y;
        const int x_241 = map[((x_234 - 2) + (x_237 * 16))];
        x_242 = (x_241 == 0);
        x_243_phi = x_242;
      }
      if (x_243_phi) {
        d = (d - 1);
        const int x_249 = p.x;
        const int x_251 = p.y;
        map[(x_249 + (x_251 * 16))] = 1;
        const int x_256 = p.x;
        const int x_259 = p.y;
        map[((x_256 - 1) + (x_259 * 16))] = 1;
        const int x_264 = p.x;
        const int x_267 = p.y;
        map[((x_264 - 2) + (x_267 * 16))] = 1;
        const int x_272 = p.x;
        p.x = (x_272 - 2);
      }
      const bool x_276 = (d >= 0);
      x_282_phi = x_276;
      if (x_276) {
        const int x_280 = p.y;
        x_281 = (x_280 > 0);
        x_282_phi = x_281;
      }
      const bool x_282 = x_282_phi;
      x_296_phi = x_282;
      if (x_282) {
        const int x_286 = p.x;
        const int x_288 = p.y;
        const int x_291[256] = map;
        const int tint_symbol_4[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        map = tint_symbol_4;
        map = x_291;
        const int x_294 = map[(x_286 + ((x_288 - 2) * 16))];
        x_295 = (x_294 == 0);
        x_296_phi = x_295;
      }
      if (x_296_phi) {
        d = (d - 1);
        const int x_302 = p.x;
        const int x_304 = p.y;
        map[(x_302 + (x_304 * 16))] = 1;
        const int x_309 = p.x;
        const int x_311 = p.y;
        map[(x_309 + ((x_311 - 1) * 16))] = 1;
        const int x_317 = p.x;
        const int x_319 = p.y;
        const int x_321[256] = map;
        const int tint_symbol_5[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        map = tint_symbol_5;
        map = x_321;
        map[(x_317 + ((x_319 - 2) * 16))] = 1;
        const int x_326 = p.y;
        p.y = (x_326 - 2);
      }
      const bool x_330 = (d >= 0);
      x_336_phi = x_330;
      if (x_330) {
        const int x_334 = p.x;
        x_335 = (x_334 < 14);
        x_336_phi = x_335;
      }
      const bool x_336 = x_336_phi;
      x_349_phi = x_336;
      if (x_336) {
        const int x_340 = p.x;
        const int x_343 = p.y;
        const int x_347 = map[((x_340 + 2) + (x_343 * 16))];
        x_348 = (x_347 == 0);
        x_349_phi = x_348;
      }
      if (x_349_phi) {
        d = (d - 1);
        const int x_355 = p.x;
        const int x_357 = p.y;
        map[(x_355 + (x_357 * 16))] = 1;
        const int x_362 = p.x;
        const int x_365 = p.y;
        map[((x_362 + 1) + (x_365 * 16))] = 1;
        const int x_370 = p.x;
        const int x_373 = p.y;
        map[((x_370 + 2) + (x_373 * 16))] = 1;
        const int x_378 = p.x;
        p.x = (x_378 + 2);
      }
      const bool x_382 = (d >= 0);
      x_388_phi = x_382;
      if (x_382) {
        const int x_386 = p.y;
        x_387 = (x_386 < 14);
        x_388_phi = x_387;
      }
      const bool x_388 = x_388_phi;
      x_401_phi = x_388;
      if (x_388) {
        const int x_392 = p.x;
        const int x_394 = p.y;
        const int x_399 = map[(x_392 + ((x_394 + 2) * 16))];
        x_400 = (x_399 == 0);
        x_401_phi = x_400;
      }
      if (x_401_phi) {
        d = (d - 1);
        const int x_407 = p.x;
        const int x_409 = p.y;
        map[(x_407 + (x_409 * 16))] = 1;
        const int x_414 = p.x;
        const int x_416 = p.y;
        map[(x_414 + ((x_416 + 1) * 16))] = 1;
        const int x_422 = p.x;
        const int x_424 = p.y;
        map[(x_422 + ((x_424 + 2) * 16))] = 1;
        const int x_430 = p.y;
        p.y = (x_430 + 2);
      }
    }
    const int x_434 = ipos.y;
    const int x_437 = ipos.x;
    const int x_440 = map[((x_434 * 16) + x_437)];
    if ((x_440 == 1)) {
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
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
