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
  vec4 x_57 = tint_symbol;
  vec2 x_60 = v_1.tint_symbol_3.resolution;
  pos = (vec2(x_57[0u], x_57[1u]) / x_60);
  float x_63 = pos.x;
  float x_67 = pos.y;
  int v_3 = tint_f32_to_i32((x_63 * 16.0f));
  ipos = ivec2(v_3, tint_f32_to_i32((x_67 * 16.0f)));
  i = 0;
  {
    while(true) {
      int x_75 = i;
      if ((x_75 < 256)) {
      } else {
        break;
      }
      int x_78 = i;
      map[x_78] = 0;
      {
        int x_80 = i;
        i = (x_80 + 1);
      }
      continue;
    }
  }
  p = ivec2(0);
  canwalk = true;
  v = 0;
  {
    while(true) {
      bool x_102 = false;
      bool x_122 = false;
      bool x_142 = false;
      bool x_162 = false;
      bool x_103_phi = false;
      bool x_123_phi = false;
      bool x_143_phi = false;
      bool x_163_phi = false;
      int x_86 = v;
      v = (x_86 + 1);
      directions = 0;
      int x_89 = p.x;
      bool x_90 = (x_89 > 0);
      x_103_phi = x_90;
      if (x_90) {
        int x_94 = p.x;
        int x_97 = p.y;
        int x_101 = map[((x_94 - 2) + (x_97 * 16))];
        x_102 = (x_101 == 0);
        x_103_phi = x_102;
      }
      bool x_103 = x_103_phi;
      if (x_103) {
        int x_106 = directions;
        directions = (x_106 + 1);
      }
      int x_109 = p.y;
      bool x_110 = (x_109 > 0);
      x_123_phi = x_110;
      if (x_110) {
        int x_114 = p.x;
        int x_116 = p.y;
        int x_121 = map[(x_114 + ((x_116 - 2) * 16))];
        x_122 = (x_121 == 0);
        x_123_phi = x_122;
      }
      bool x_123 = x_123_phi;
      if (x_123) {
        int x_126 = directions;
        directions = (x_126 + 1);
      }
      int x_129 = p.x;
      bool x_130 = (x_129 < 14);
      x_143_phi = x_130;
      if (x_130) {
        int x_134 = p.x;
        int x_137 = p.y;
        int x_141 = map[((x_134 + 2) + (x_137 * 16))];
        x_142 = (x_141 == 0);
        x_143_phi = x_142;
      }
      bool x_143 = x_143_phi;
      if (x_143) {
        int x_146 = directions;
        directions = (x_146 + 1);
      }
      int x_149 = p.y;
      bool x_150 = (x_149 < 14);
      x_163_phi = x_150;
      if (x_150) {
        int x_154 = p.x;
        int x_156 = p.y;
        int x_161 = map[(x_154 + ((x_156 + 2) * 16))];
        x_162 = (x_161 == 0);
        x_163_phi = x_162;
      }
      bool x_163 = x_163_phi;
      if (x_163) {
        int x_166 = directions;
        directions = (x_166 + 1);
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
      int x_168 = directions;
      if ((x_168 == 0)) {
        canwalk = false;
        i = 0;
        {
          while(true) {
            int x_177 = i;
            if ((x_177 < 8)) {
            } else {
              break;
            }
            j = 0;
            {
              while(true) {
                int x_184 = j;
                if ((x_184 < 8)) {
                } else {
                  break;
                }
                int x_187 = j;
                int x_189 = i;
                int x_194 = map[((x_187 * 2) + ((x_189 * 2) * 16))];
                if ((x_194 == 0)) {
                  int x_198 = j;
                  p[0u] = (x_198 * 2);
                  int x_201 = i;
                  p[1u] = (x_201 * 2);
                  canwalk = true;
                }
                {
                  int x_204 = j;
                  j = (x_204 + 1);
                }
                continue;
              }
            }
            {
              int x_206 = i;
              i = (x_206 + 1);
            }
            continue;
          }
        }
        int x_209 = p.x;
        int x_211 = p.y;
        map[(x_209 + (x_211 * 16))] = 1;
      } else {
        int x_215 = v;
        int x_216 = directions;
        d = tint_mod_i32(x_215, x_216);
        int x_218 = directions;
        int x_219 = v;
        v = (x_219 + x_218);
        int x_221 = d;
        bool x_222 = (x_221 >= 0);
        x_228_phi = x_222;
        if (x_222) {
          int x_226 = p.x;
          x_227 = (x_226 > 0);
          x_228_phi = x_227;
        }
        bool x_228 = x_228_phi;
        x_241_phi = x_228;
        if (x_228) {
          int x_232 = p.x;
          int x_235 = p.y;
          int x_239 = map[((x_232 - 2) + (x_235 * 16))];
          x_240 = (x_239 == 0);
          x_241_phi = x_240;
        }
        bool x_241 = x_241_phi;
        if (x_241) {
          int x_244 = d;
          d = (x_244 - 1);
          int x_247 = p.x;
          int x_249 = p.y;
          map[(x_247 + (x_249 * 16))] = 1;
          int x_254 = p.x;
          int x_257 = p.y;
          map[((x_254 - 1) + (x_257 * 16))] = 1;
          int x_262 = p.x;
          int x_265 = p.y;
          map[((x_262 - 2) + (x_265 * 16))] = 1;
          int x_270 = p.x;
          p[0u] = (x_270 - 2);
        }
        int x_273 = d;
        bool x_274 = (x_273 >= 0);
        x_280_phi = x_274;
        if (x_274) {
          int x_278 = p.y;
          x_279 = (x_278 > 0);
          x_280_phi = x_279;
        }
        bool x_280 = x_280_phi;
        x_293_phi = x_280;
        if (x_280) {
          int x_284 = p.x;
          int x_286 = p.y;
          int x_291 = map[(x_284 + ((x_286 - 2) * 16))];
          x_292 = (x_291 == 0);
          x_293_phi = x_292;
        }
        bool x_293 = x_293_phi;
        if (x_293) {
          int x_296 = d;
          d = (x_296 - 1);
          int x_299 = p.x;
          int x_301 = p.y;
          map[(x_299 + (x_301 * 16))] = 1;
          int x_306 = p.x;
          int x_308 = p.y;
          map[(x_306 + ((x_308 - 1) * 16))] = 1;
          int x_314 = p.x;
          int x_316 = p.y;
          map[(x_314 + ((x_316 - 2) * 16))] = 1;
          int x_322 = p.y;
          p[1u] = (x_322 - 2);
        }
        int x_325 = d;
        bool x_326 = (x_325 >= 0);
        x_332_phi = x_326;
        if (x_326) {
          int x_330 = p.x;
          x_331 = (x_330 < 14);
          x_332_phi = x_331;
        }
        bool x_332 = x_332_phi;
        x_345_phi = x_332;
        if (x_332) {
          int x_336 = p.x;
          int x_339 = p.y;
          int x_343 = map[((x_336 + 2) + (x_339 * 16))];
          x_344 = (x_343 == 0);
          x_345_phi = x_344;
        }
        bool x_345 = x_345_phi;
        if (x_345) {
          int x_348 = d;
          d = (x_348 - 1);
          int x_351 = p.x;
          int x_353 = p.y;
          map[(x_351 + (x_353 * 16))] = 1;
          int x_358 = p.x;
          int x_361 = p.y;
          map[((x_358 + 1) + (x_361 * 16))] = 1;
          int x_366 = p.x;
          int x_369 = p.y;
          map[((x_366 + 2) + (x_369 * 16))] = 1;
          int x_374 = p.x;
          p[0u] = (x_374 + 2);
        }
        int x_377 = d;
        bool x_378 = (x_377 >= 0);
        x_384_phi = x_378;
        if (x_378) {
          int x_382 = p.y;
          x_383 = (x_382 < 14);
          x_384_phi = x_383;
        }
        bool x_384 = x_384_phi;
        x_397_phi = x_384;
        if (x_384) {
          int x_388 = p.x;
          int x_390 = p.y;
          int x_395 = map[(x_388 + ((x_390 + 2) * 16))];
          x_396 = (x_395 == 0);
          x_397_phi = x_396;
        }
        bool x_397 = x_397_phi;
        if (x_397) {
          int x_400 = d;
          d = (x_400 - 1);
          int x_403 = p.x;
          int x_405 = p.y;
          map[(x_403 + (x_405 * 16))] = 1;
          int x_410 = p.x;
          int x_412 = p.y;
          map[(x_410 + ((x_412 + 1) * 16))] = 1;
          int x_418 = p.x;
          int x_420 = p.y;
          map[(x_418 + ((x_420 + 2) * 16))] = 1;
          int x_426 = p.y;
          p[1u] = (x_426 + 2);
        }
      }
      int x_430 = ipos.y;
      int x_433 = ipos.x;
      int x_436 = map[((x_430 * 16) + x_433)];
      if ((x_436 == 1)) {
        x_GLF_color = vec4(1.0f);
        return;
      }
      {
        bool x_440 = canwalk;
        if (!(x_440)) { break; }
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
