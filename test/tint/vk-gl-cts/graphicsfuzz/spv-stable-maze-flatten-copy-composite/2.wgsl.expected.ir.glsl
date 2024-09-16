SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};

vec4 tint_symbol = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
int map[256] = int[256](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_2 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_2) * v_2));
}
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  vec2 pos = vec2(0.0f);
  ivec2 ipos = ivec2(0);
  int i = 0;
  ivec2 p = ivec2(0);
  bool canwalk = false;
  int v = 0;
  int directions = 0;
  int j = 0;
  int d = 0;
  vec4 x_59 = tint_symbol;
  vec2 x_62 = v_1.tint_symbol_3.resolution;
  pos = (vec2(x_59[0u], x_59[1u]) / x_62);
  float x_65 = pos.x;
  float x_69 = pos.y;
  int v_3 = tint_f32_to_i32((x_65 * 16.0f));
  ipos = ivec2(v_3, tint_f32_to_i32((x_69 * 16.0f)));
  i = 0;
  {
    while(true) {
      int x_77 = i;
      if ((x_77 < 256)) {
      } else {
        break;
      }
      int x_80 = i;
      map[x_80] = 0;
      {
        int x_82 = i;
        i = (x_82 + 1);
      }
      continue;
    }
  }
  p = ivec2(0);
  canwalk = true;
  v = 0;
  {
    while(true) {
      bool x_104 = false;
      bool x_124 = false;
      bool x_144 = false;
      bool x_164 = false;
      bool x_105_phi = false;
      bool x_125_phi = false;
      bool x_145_phi = false;
      bool x_165_phi = false;
      int x_88 = v;
      v = (x_88 + 1);
      directions = 0;
      int x_91 = p.x;
      bool x_92 = (x_91 > 0);
      x_105_phi = x_92;
      if (x_92) {
        int x_96 = p.x;
        int x_99 = p.y;
        int x_103 = map[((x_96 - 2) + (x_99 * 16))];
        x_104 = (x_103 == 0);
        x_105_phi = x_104;
      }
      bool x_105 = x_105_phi;
      if (x_105) {
        int x_108 = directions;
        directions = (x_108 + 1);
      }
      int x_111 = p.y;
      bool x_112 = (x_111 > 0);
      x_125_phi = x_112;
      if (x_112) {
        int x_116 = p.x;
        int x_118 = p.y;
        int x_123 = map[(x_116 + ((x_118 - 2) * 16))];
        x_124 = (x_123 == 0);
        x_125_phi = x_124;
      }
      bool x_125 = x_125_phi;
      if (x_125) {
        int x_128 = directions;
        directions = (x_128 + 1);
      }
      int x_131 = p.x;
      bool x_132 = (x_131 < 14);
      x_145_phi = x_132;
      if (x_132) {
        int x_136 = p.x;
        int x_139 = p.y;
        int x_143 = map[((x_136 + 2) + (x_139 * 16))];
        x_144 = (x_143 == 0);
        x_145_phi = x_144;
      }
      bool x_145 = x_145_phi;
      if (x_145) {
        int x_148 = directions;
        directions = (x_148 + 1);
      }
      int x_151 = p.y;
      bool x_152 = (x_151 < 14);
      x_165_phi = x_152;
      if (x_152) {
        int x_156 = p.x;
        int x_158 = p.y;
        int x_163 = map[(x_156 + ((x_158 + 2) * 16))];
        x_164 = (x_163 == 0);
        x_165_phi = x_164;
      }
      bool x_165 = x_165_phi;
      if (x_165) {
        int x_168 = directions;
        directions = (x_168 + 1);
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
      int x_170 = directions;
      if ((x_170 == 0)) {
        canwalk = false;
        i = 0;
        {
          while(true) {
            int x_179 = i;
            if ((x_179 < 8)) {
            } else {
              break;
            }
            j = 0;
            {
              while(true) {
                int x_186 = j;
                if ((x_186 < 8)) {
                } else {
                  break;
                }
                int x_189 = j;
                int x_191 = i;
                int x_196 = map[((x_189 * 2) + ((x_191 * 2) * 16))];
                if ((x_196 == 0)) {
                  int x_200 = j;
                  p[0u] = (x_200 * 2);
                  int x_203 = i;
                  p[1u] = (x_203 * 2);
                  canwalk = true;
                }
                {
                  int x_206 = j;
                  j = (x_206 + 1);
                }
                continue;
              }
            }
            {
              int x_208 = i;
              i = (x_208 + 1);
            }
            continue;
          }
        }
        int x_211 = p.x;
        int x_213 = p.y;
        map[(x_211 + (x_213 * 16))] = 1;
      } else {
        int x_217 = v;
        int x_218 = directions;
        d = tint_mod_i32(x_217, x_218);
        int x_220 = directions;
        int x_221 = v;
        v = (x_221 + x_220);
        int x_223 = d;
        bool x_224 = (x_223 >= 0);
        x_230_phi = x_224;
        if (x_224) {
          int x_228 = p.x;
          x_229 = (x_228 > 0);
          x_230_phi = x_229;
        }
        bool x_230 = x_230_phi;
        x_243_phi = x_230;
        if (x_230) {
          int x_234 = p.x;
          int x_237 = p.y;
          int x_241 = map[((x_234 - 2) + (x_237 * 16))];
          x_242 = (x_241 == 0);
          x_243_phi = x_242;
        }
        bool x_243 = x_243_phi;
        if (x_243) {
          int x_246 = d;
          d = (x_246 - 1);
          int x_249 = p.x;
          int x_251 = p.y;
          map[(x_249 + (x_251 * 16))] = 1;
          int x_256 = p.x;
          int x_259 = p.y;
          map[((x_256 - 1) + (x_259 * 16))] = 1;
          int x_264 = p.x;
          int x_267 = p.y;
          map[((x_264 - 2) + (x_267 * 16))] = 1;
          int x_272 = p.x;
          p[0u] = (x_272 - 2);
        }
        int x_275 = d;
        bool x_276 = (x_275 >= 0);
        x_282_phi = x_276;
        if (x_276) {
          int x_280 = p.y;
          x_281 = (x_280 > 0);
          x_282_phi = x_281;
        }
        bool x_282 = x_282_phi;
        x_296_phi = x_282;
        if (x_282) {
          int x_286 = p.x;
          int x_288 = p.y;
          int x_291[256] = map;
          map = int[256](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          map = x_291;
          int x_294 = map[(x_286 + ((x_288 - 2) * 16))];
          x_295 = (x_294 == 0);
          x_296_phi = x_295;
        }
        bool x_296 = x_296_phi;
        if (x_296) {
          int x_299 = d;
          d = (x_299 - 1);
          int x_302 = p.x;
          int x_304 = p.y;
          map[(x_302 + (x_304 * 16))] = 1;
          int x_309 = p.x;
          int x_311 = p.y;
          map[(x_309 + ((x_311 - 1) * 16))] = 1;
          int x_317 = p.x;
          int x_319 = p.y;
          int x_321[256] = map;
          map = int[256](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          map = x_321;
          map[(x_317 + ((x_319 - 2) * 16))] = 1;
          int x_326 = p.y;
          p[1u] = (x_326 - 2);
        }
        int x_329 = d;
        bool x_330 = (x_329 >= 0);
        x_336_phi = x_330;
        if (x_330) {
          int x_334 = p.x;
          x_335 = (x_334 < 14);
          x_336_phi = x_335;
        }
        bool x_336 = x_336_phi;
        x_349_phi = x_336;
        if (x_336) {
          int x_340 = p.x;
          int x_343 = p.y;
          int x_347 = map[((x_340 + 2) + (x_343 * 16))];
          x_348 = (x_347 == 0);
          x_349_phi = x_348;
        }
        bool x_349 = x_349_phi;
        if (x_349) {
          int x_352 = d;
          d = (x_352 - 1);
          int x_355 = p.x;
          int x_357 = p.y;
          map[(x_355 + (x_357 * 16))] = 1;
          int x_362 = p.x;
          int x_365 = p.y;
          map[((x_362 + 1) + (x_365 * 16))] = 1;
          int x_370 = p.x;
          int x_373 = p.y;
          map[((x_370 + 2) + (x_373 * 16))] = 1;
          int x_378 = p.x;
          p[0u] = (x_378 + 2);
        }
        int x_381 = d;
        bool x_382 = (x_381 >= 0);
        x_388_phi = x_382;
        if (x_382) {
          int x_386 = p.y;
          x_387 = (x_386 < 14);
          x_388_phi = x_387;
        }
        bool x_388 = x_388_phi;
        x_401_phi = x_388;
        if (x_388) {
          int x_392 = p.x;
          int x_394 = p.y;
          int x_399 = map[(x_392 + ((x_394 + 2) * 16))];
          x_400 = (x_399 == 0);
          x_401_phi = x_400;
        }
        bool x_401 = x_401_phi;
        if (x_401) {
          int x_404 = d;
          d = (x_404 - 1);
          int x_407 = p.x;
          int x_409 = p.y;
          map[(x_407 + (x_409 * 16))] = 1;
          int x_414 = p.x;
          int x_416 = p.y;
          map[(x_414 + ((x_416 + 1) * 16))] = 1;
          int x_422 = p.x;
          int x_424 = p.y;
          map[(x_422 + ((x_424 + 2) * 16))] = 1;
          int x_430 = p.y;
          p[1u] = (x_430 + 2);
        }
      }
      int x_434 = ipos.y;
      int x_437 = ipos.x;
      int x_440 = map[((x_434 * 16) + x_437)];
      if ((x_440 == 1)) {
        x_GLF_color = vec4(1.0f);
        return;
      }
      {
        bool x_444 = canwalk;
        if (!(x_444)) { break; }
      }
      continue;
    }
  }
  x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:23: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:23: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:23: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
