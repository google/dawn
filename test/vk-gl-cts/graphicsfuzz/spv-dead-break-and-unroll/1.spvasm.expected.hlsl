static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static int map[256] = (int[256])0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float2x4 x_60 = float2x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));

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
  const float4 x_63 = gl_FragCoord;
  const float2 x_67 = asfloat(x_7[0].xy);
  const int x_68 = -((256 - 14));
  pos = (float2(x_63.x, x_63.y) / x_67);
  const float x_71 = pos.x;
  const float x_75 = pos.y;
  ipos = int2(int((x_71 * 16.0f)), int((x_75 * 16.0f)));
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
    bool x_110 = false;
    bool x_130 = false;
    bool x_150 = false;
    bool x_171 = false;
    bool x_111_phi = false;
    bool x_131_phi = false;
    bool x_151_phi = false;
    bool x_172_phi = false;
    v = (v + 1);
    directions = 0;
    const int x_97 = p.x;
    const bool x_98 = (x_97 > 0);
    x_111_phi = x_98;
    if (x_98) {
      const int x_102 = p.x;
      const int x_105 = p.y;
      const int x_109 = map[((x_102 - 2) + (x_105 * 16))];
      x_110 = (x_109 == 0);
      x_111_phi = x_110;
    }
    if (x_111_phi) {
      directions = (directions + 1);
    }
    const int x_117 = p.y;
    const bool x_118 = (x_117 > 0);
    x_131_phi = x_118;
    if (x_118) {
      const int x_122 = p.x;
      const int x_124 = p.y;
      const int x_129 = map[(x_122 + ((x_124 - 2) * 16))];
      x_130 = (x_129 == 0);
      x_131_phi = x_130;
    }
    if (x_131_phi) {
      directions = (directions + 1);
    }
    const int x_137 = p.x;
    const bool x_138 = (x_137 < 14);
    x_151_phi = x_138;
    if (x_138) {
      const int x_142 = p.x;
      const int x_145 = p.y;
      const int x_149 = map[((x_142 + 2) + (x_145 * 16))];
      x_150 = (x_149 == 0);
      x_151_phi = x_150;
    }
    if (x_151_phi) {
      directions = (directions + 1);
    }
    const int x_156 = (256 - x_68);
    const int x_158 = p.y;
    const bool x_159 = (x_158 < 14);
    x_172_phi = x_159;
    if (x_159) {
      const int x_163 = p.x;
      const int x_165 = p.y;
      const int x_170 = map[(x_163 + ((x_165 + 2) * 16))];
      x_171 = (x_170 == 0);
      x_172_phi = x_171;
    }
    if (x_172_phi) {
      directions = (directions + 1);
    }
    bool x_237 = false;
    bool x_250 = false;
    bool x_289 = false;
    bool x_302 = false;
    bool x_341 = false;
    bool x_354 = false;
    bool x_393 = false;
    bool x_406 = false;
    bool x_238_phi = false;
    bool x_251_phi = false;
    bool x_290_phi = false;
    bool x_303_phi = false;
    bool x_342_phi = false;
    bool x_355_phi = false;
    bool x_394_phi = false;
    bool x_407_phi = false;
    if ((directions == 0)) {
      canwalk = false;
      i = 0;
      while (true) {
        const int x_186 = i;
        if ((x_186 < 8)) {
        } else {
          break;
        }
        j = 0;
        const int x_189 = (x_156 - x_186);
        x_60 = float2x4(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
        if (false) {
          {
            i = (i + 1);
          }
          continue;
        }
        {
          for(; (j < 8); j = (j + 1)) {
            const int x_204 = map[((j * 2) + ((i * 2) * 16))];
            if ((x_204 == 0)) {
              p.x = (j * 2);
              p.y = (i * 2);
              canwalk = true;
            }
          }
        }
        {
          i = (i + 1);
        }
      }
      const int x_219 = p.x;
      const int x_221 = p.y;
      map[(x_219 + (x_221 * 16))] = 1;
    } else {
      d = (v % directions);
      v = (v + directions);
      const bool x_232 = (d >= 0);
      x_238_phi = x_232;
      if (x_232) {
        const int x_236 = p.x;
        x_237 = (x_236 > 0);
        x_238_phi = x_237;
      }
      const bool x_238 = x_238_phi;
      x_251_phi = x_238;
      if (x_238) {
        const int x_242 = p.x;
        const int x_245 = p.y;
        const int x_249 = map[((x_242 - 2) + (x_245 * 16))];
        x_250 = (x_249 == 0);
        x_251_phi = x_250;
      }
      if (x_251_phi) {
        d = (d - 1);
        const int x_257 = p.x;
        const int x_259 = p.y;
        map[(x_257 + (x_259 * 16))] = 1;
        const int x_264 = p.x;
        const int x_267 = p.y;
        map[((x_264 - 1) + (x_267 * 16))] = 1;
        const int x_272 = p.x;
        const int x_275 = p.y;
        map[((x_272 - 2) + (x_275 * 16))] = 1;
        const int x_280 = p.x;
        p.x = (x_280 - 2);
      }
      const bool x_284 = (d >= 0);
      x_290_phi = x_284;
      if (x_284) {
        const int x_288 = p.y;
        x_289 = (x_288 > 0);
        x_290_phi = x_289;
      }
      const bool x_290 = x_290_phi;
      x_303_phi = x_290;
      if (x_290) {
        const int x_294 = p.x;
        const int x_296 = p.y;
        const int x_301 = map[(x_294 + ((x_296 - 2) * 16))];
        x_302 = (x_301 == 0);
        x_303_phi = x_302;
      }
      if (x_303_phi) {
        d = (d - 1);
        const int x_309 = p.x;
        const int x_311 = p.y;
        map[(x_309 + (x_311 * 16))] = 1;
        const int x_316 = p.x;
        const int x_318 = p.y;
        map[(x_316 + ((x_318 - 1) * 16))] = 1;
        const int x_324 = p.x;
        const int x_326 = p.y;
        map[(x_324 + ((x_326 - 2) * 16))] = 1;
        const int x_332 = p.y;
        p.y = (x_332 - 2);
      }
      const bool x_336 = (d >= 0);
      x_342_phi = x_336;
      if (x_336) {
        const int x_340 = p.x;
        x_341 = (x_340 < 14);
        x_342_phi = x_341;
      }
      const bool x_342 = x_342_phi;
      x_355_phi = x_342;
      if (x_342) {
        const int x_346 = p.x;
        const int x_349 = p.y;
        const int x_353 = map[((x_346 + 2) + (x_349 * 16))];
        x_354 = (x_353 == 0);
        x_355_phi = x_354;
      }
      if (x_355_phi) {
        d = (d - 1);
        const int x_361 = p.x;
        const int x_363 = p.y;
        map[(x_361 + (x_363 * 16))] = 1;
        const int x_368 = p.x;
        const int x_371 = p.y;
        map[((x_368 + 1) + (x_371 * 16))] = 1;
        const int x_376 = p.x;
        const int x_379 = p.y;
        map[((x_376 + 2) + (x_379 * 16))] = 1;
        const int x_384 = p.x;
        p.x = (x_384 + 2);
      }
      const bool x_388 = (d >= 0);
      x_394_phi = x_388;
      if (x_388) {
        const int x_392 = p.y;
        x_393 = (x_392 < 14);
        x_394_phi = x_393;
      }
      const bool x_394 = x_394_phi;
      x_407_phi = x_394;
      if (x_394) {
        const int x_398 = p.x;
        const int x_400 = p.y;
        const int x_405 = map[(x_398 + ((x_400 + 2) * 16))];
        x_406 = (x_405 == 0);
        x_407_phi = x_406;
      }
      if (x_407_phi) {
        d = (d - 1);
        const int x_413 = p.x;
        const int x_415 = p.y;
        map[(x_413 + (x_415 * 16))] = 1;
        const int x_420 = p.x;
        const int x_422 = p.y;
        map[(x_420 + ((x_422 + 1) * 16))] = 1;
        const int x_428 = p.x;
        const int x_430 = p.y;
        map[(x_428 + ((x_430 + 2) * 16))] = 1;
        const int x_436 = p.y;
        p.y = (x_436 + 2);
      }
    }
    const int x_440 = ipos.y;
    const int x_443 = ipos.x;
    const int x_446 = map[((x_440 * 16) + x_443)];
    if ((x_446 == 1)) {
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
