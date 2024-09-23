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
mat2x4 x_60 = mat2x4(vec4(0.0f), vec4(0.0f));
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
  vec4 x_63 = tint_symbol;
  vec2 x_67 = v_1.tint_symbol_3.resolution;
  int x_68 = -242;
  pos = (vec2(x_63[0u], x_63[1u]) / x_67);
  float x_71 = pos.x;
  float x_75 = pos.y;
  int v_3 = tint_f32_to_i32((x_71 * 16.0f));
  ipos = ivec2(v_3, tint_f32_to_i32((x_75 * 16.0f)));
  i = 0;
  {
    while(true) {
      int x_83 = i;
      if ((x_83 < 256)) {
      } else {
        break;
      }
      int x_86 = i;
      map[x_86] = 0;
      {
        int x_88 = i;
        i = (x_88 + 1);
      }
      continue;
    }
  }
  p = ivec2(0);
  canwalk = true;
  v = 0;
  {
    while(true) {
      bool x_110 = false;
      bool x_130 = false;
      bool x_150 = false;
      bool x_171 = false;
      bool x_111_phi = false;
      bool x_131_phi = false;
      bool x_151_phi = false;
      bool x_172_phi = false;
      int x_94 = v;
      v = (x_94 + 1);
      directions = 0;
      int x_97 = p.x;
      bool x_98 = (x_97 > 0);
      x_111_phi = x_98;
      if (x_98) {
        int x_102 = p.x;
        int x_105 = p.y;
        int x_109 = map[((x_102 - 2) + (x_105 * 16))];
        x_110 = (x_109 == 0);
        x_111_phi = x_110;
      }
      bool x_111 = x_111_phi;
      if (x_111) {
        int x_114 = directions;
        directions = (x_114 + 1);
      }
      int x_117 = p.y;
      bool x_118 = (x_117 > 0);
      x_131_phi = x_118;
      if (x_118) {
        int x_122 = p.x;
        int x_124 = p.y;
        int x_129 = map[(x_122 + ((x_124 - 2) * 16))];
        x_130 = (x_129 == 0);
        x_131_phi = x_130;
      }
      bool x_131 = x_131_phi;
      if (x_131) {
        int x_134 = directions;
        directions = (x_134 + 1);
      }
      int x_137 = p.x;
      bool x_138 = (x_137 < 14);
      x_151_phi = x_138;
      if (x_138) {
        int x_142 = p.x;
        int x_145 = p.y;
        int x_149 = map[((x_142 + 2) + (x_145 * 16))];
        x_150 = (x_149 == 0);
        x_151_phi = x_150;
      }
      bool x_151 = x_151_phi;
      if (x_151) {
        int x_154 = directions;
        directions = (x_154 + 1);
      }
      int x_156 = (256 - x_68);
      int x_158 = p.y;
      bool x_159 = (x_158 < 14);
      x_172_phi = x_159;
      if (x_159) {
        int x_163 = p.x;
        int x_165 = p.y;
        int x_170 = map[(x_163 + ((x_165 + 2) * 16))];
        x_171 = (x_170 == 0);
        x_172_phi = x_171;
      }
      bool x_172 = x_172_phi;
      if (x_172) {
        int x_175 = directions;
        directions = (x_175 + 1);
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
      int x_177 = directions;
      if ((x_177 == 0)) {
        canwalk = false;
        i = 0;
        {
          while(true) {
            int x_186 = i;
            if ((x_186 < 8)) {
            } else {
              break;
            }
            j = 0;
            int x_189 = (x_156 - x_186);
            x_60 = mat2x4(vec4(0.0f), vec4(0.0f));
            if (false) {
              {
                int x_216 = i;
                i = (x_216 + 1);
              }
              continue;
            }
            {
              while(true) {
                int x_194 = j;
                if ((x_194 < 8)) {
                } else {
                  break;
                }
                int x_197 = j;
                int x_199 = i;
                int x_204 = map[((x_197 * 2) + ((x_199 * 2) * 16))];
                if ((x_204 == 0)) {
                  int x_208 = j;
                  p[0u] = (x_208 * 2);
                  int x_211 = i;
                  p[1u] = (x_211 * 2);
                  canwalk = true;
                }
                {
                  int x_214 = j;
                  j = (x_214 + 1);
                }
                continue;
              }
            }
            {
              int x_216 = i;
              i = (x_216 + 1);
            }
            continue;
          }
        }
        int x_219 = p.x;
        int x_221 = p.y;
        map[(x_219 + (x_221 * 16))] = 1;
      } else {
        int x_225 = v;
        int x_226 = directions;
        d = tint_mod_i32(x_225, x_226);
        int x_228 = directions;
        int x_229 = v;
        v = (x_229 + x_228);
        int x_231 = d;
        bool x_232 = (x_231 >= 0);
        x_238_phi = x_232;
        if (x_232) {
          int x_236 = p.x;
          x_237 = (x_236 > 0);
          x_238_phi = x_237;
        }
        bool x_238 = x_238_phi;
        x_251_phi = x_238;
        if (x_238) {
          int x_242 = p.x;
          int x_245 = p.y;
          int x_249 = map[((x_242 - 2) + (x_245 * 16))];
          x_250 = (x_249 == 0);
          x_251_phi = x_250;
        }
        bool x_251 = x_251_phi;
        if (x_251) {
          int x_254 = d;
          d = (x_254 - 1);
          int x_257 = p.x;
          int x_259 = p.y;
          map[(x_257 + (x_259 * 16))] = 1;
          int x_264 = p.x;
          int x_267 = p.y;
          map[((x_264 - 1) + (x_267 * 16))] = 1;
          int x_272 = p.x;
          int x_275 = p.y;
          map[((x_272 - 2) + (x_275 * 16))] = 1;
          int x_280 = p.x;
          p[0u] = (x_280 - 2);
        }
        int x_283 = d;
        bool x_284 = (x_283 >= 0);
        x_290_phi = x_284;
        if (x_284) {
          int x_288 = p.y;
          x_289 = (x_288 > 0);
          x_290_phi = x_289;
        }
        bool x_290 = x_290_phi;
        x_303_phi = x_290;
        if (x_290) {
          int x_294 = p.x;
          int x_296 = p.y;
          int x_301 = map[(x_294 + ((x_296 - 2) * 16))];
          x_302 = (x_301 == 0);
          x_303_phi = x_302;
        }
        bool x_303 = x_303_phi;
        if (x_303) {
          int x_306 = d;
          d = (x_306 - 1);
          int x_309 = p.x;
          int x_311 = p.y;
          map[(x_309 + (x_311 * 16))] = 1;
          int x_316 = p.x;
          int x_318 = p.y;
          map[(x_316 + ((x_318 - 1) * 16))] = 1;
          int x_324 = p.x;
          int x_326 = p.y;
          map[(x_324 + ((x_326 - 2) * 16))] = 1;
          int x_332 = p.y;
          p[1u] = (x_332 - 2);
        }
        int x_335 = d;
        bool x_336 = (x_335 >= 0);
        x_342_phi = x_336;
        if (x_336) {
          int x_340 = p.x;
          x_341 = (x_340 < 14);
          x_342_phi = x_341;
        }
        bool x_342 = x_342_phi;
        x_355_phi = x_342;
        if (x_342) {
          int x_346 = p.x;
          int x_349 = p.y;
          int x_353 = map[((x_346 + 2) + (x_349 * 16))];
          x_354 = (x_353 == 0);
          x_355_phi = x_354;
        }
        bool x_355 = x_355_phi;
        if (x_355) {
          int x_358 = d;
          d = (x_358 - 1);
          int x_361 = p.x;
          int x_363 = p.y;
          map[(x_361 + (x_363 * 16))] = 1;
          int x_368 = p.x;
          int x_371 = p.y;
          map[((x_368 + 1) + (x_371 * 16))] = 1;
          int x_376 = p.x;
          int x_379 = p.y;
          map[((x_376 + 2) + (x_379 * 16))] = 1;
          int x_384 = p.x;
          p[0u] = (x_384 + 2);
        }
        int x_387 = d;
        bool x_388 = (x_387 >= 0);
        x_394_phi = x_388;
        if (x_388) {
          int x_392 = p.y;
          x_393 = (x_392 < 14);
          x_394_phi = x_393;
        }
        bool x_394 = x_394_phi;
        x_407_phi = x_394;
        if (x_394) {
          int x_398 = p.x;
          int x_400 = p.y;
          int x_405 = map[(x_398 + ((x_400 + 2) * 16))];
          x_406 = (x_405 == 0);
          x_407_phi = x_406;
        }
        bool x_407 = x_407_phi;
        if (x_407) {
          int x_410 = d;
          d = (x_410 - 1);
          int x_413 = p.x;
          int x_415 = p.y;
          map[(x_413 + (x_415 * 16))] = 1;
          int x_420 = p.x;
          int x_422 = p.y;
          map[(x_420 + ((x_422 + 1) * 16))] = 1;
          int x_428 = p.x;
          int x_430 = p.y;
          map[(x_428 + ((x_430 + 2) * 16))] = 1;
          int x_436 = p.y;
          p[1u] = (x_436 + 2);
        }
      }
      int x_440 = ipos.y;
      int x_443 = ipos.x;
      int x_446 = map[((x_440 * 16) + x_443)];
      if ((x_446 == 1)) {
        x_GLF_color = vec4(1.0f);
        return;
      }
      {
        bool x_450 = canwalk;
        if (!(x_450)) { break; }
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
ERROR: 0:24: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:24: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:24: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
