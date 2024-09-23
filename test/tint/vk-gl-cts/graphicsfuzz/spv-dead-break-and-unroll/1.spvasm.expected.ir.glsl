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
  int x_68 = -242;
  pos = (tint_symbol.xy / v_1.tint_symbol_3.resolution);
  int v_3 = tint_f32_to_i32((pos.x * 16.0f));
  ipos = ivec2(v_3, tint_f32_to_i32((pos.y * 16.0f)));
  i = 0;
  {
    while(true) {
      if ((i < 256)) {
      } else {
        break;
      }
      int x_86 = i;
      map[x_86] = 0;
      {
        i = (i + 1);
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
      bool x_111 = false;
      bool x_130 = false;
      bool x_131 = false;
      bool x_150 = false;
      bool x_151 = false;
      bool x_171 = false;
      bool x_172 = false;
      v = (v + 1);
      directions = 0;
      bool x_98 = (p.x > 0);
      x_111 = x_98;
      if (x_98) {
        x_110 = (map[((p.x - 2) + (p.y * 16))] == 0);
        x_111 = x_110;
      }
      if (x_111) {
        directions = (directions + 1);
      }
      bool x_118 = (p.y > 0);
      x_131 = x_118;
      if (x_118) {
        x_130 = (map[(p.x + ((p.y - 2) * 16))] == 0);
        x_131 = x_130;
      }
      if (x_131) {
        directions = (directions + 1);
      }
      bool x_138 = (p.x < 14);
      x_151 = x_138;
      if (x_138) {
        x_150 = (map[((p.x + 2) + (p.y * 16))] == 0);
        x_151 = x_150;
      }
      if (x_151) {
        directions = (directions + 1);
      }
      int x_156 = (256 - x_68);
      bool x_159 = (p.y < 14);
      x_172 = x_159;
      if (x_159) {
        x_171 = (map[(p.x + ((p.y + 2) * 16))] == 0);
        x_172 = x_171;
      }
      if (x_172) {
        directions = (directions + 1);
      }
      bool x_237 = false;
      bool x_238 = false;
      bool x_250 = false;
      bool x_251 = false;
      bool x_289 = false;
      bool x_290 = false;
      bool x_302 = false;
      bool x_303 = false;
      bool x_341 = false;
      bool x_342 = false;
      bool x_354 = false;
      bool x_355 = false;
      bool x_393 = false;
      bool x_394 = false;
      bool x_406 = false;
      bool x_407 = false;
      if ((directions == 0)) {
        canwalk = false;
        i = 0;
        {
          while(true) {
            int x_186 = i;
            if ((i < 8)) {
            } else {
              break;
            }
            j = 0;
            int x_189 = (x_156 - x_186);
            x_60 = mat2x4(vec4(0.0f), vec4(0.0f));
            if (false) {
              {
                i = (i + 1);
              }
              continue;
            }
            {
              while(true) {
                if ((j < 8)) {
                } else {
                  break;
                }
                if ((map[((j * 2) + ((i * 2) * 16))] == 0)) {
                  p[0u] = (j * 2);
                  p[1u] = (i * 2);
                  canwalk = true;
                }
                {
                  j = (j + 1);
                }
                continue;
              }
            }
            {
              i = (i + 1);
            }
            continue;
          }
        }
        int x_219 = p.x;
        int x_221 = p.y;
        map[(x_219 + (x_221 * 16))] = 1;
      } else {
        d = tint_mod_i32(v, directions);
        v = (v + directions);
        bool x_232 = (d >= 0);
        x_238 = x_232;
        if (x_232) {
          x_237 = (p.x > 0);
          x_238 = x_237;
        }
        x_251 = x_238;
        if (x_238) {
          x_250 = (map[((p.x - 2) + (p.y * 16))] == 0);
          x_251 = x_250;
        }
        if (x_251) {
          d = (d - 1);
          int x_257 = p.x;
          int x_259 = p.y;
          map[(x_257 + (x_259 * 16))] = 1;
          int x_264 = p.x;
          int x_267 = p.y;
          map[((x_264 - 1) + (x_267 * 16))] = 1;
          int x_272 = p.x;
          int x_275 = p.y;
          map[((x_272 - 2) + (x_275 * 16))] = 1;
          p[0u] = (p.x - 2);
        }
        bool x_284 = (d >= 0);
        x_290 = x_284;
        if (x_284) {
          x_289 = (p.y > 0);
          x_290 = x_289;
        }
        x_303 = x_290;
        if (x_290) {
          x_302 = (map[(p.x + ((p.y - 2) * 16))] == 0);
          x_303 = x_302;
        }
        if (x_303) {
          d = (d - 1);
          int x_309 = p.x;
          int x_311 = p.y;
          map[(x_309 + (x_311 * 16))] = 1;
          int x_316 = p.x;
          int x_318 = p.y;
          map[(x_316 + ((x_318 - 1) * 16))] = 1;
          int x_324 = p.x;
          int x_326 = p.y;
          map[(x_324 + ((x_326 - 2) * 16))] = 1;
          p[1u] = (p.y - 2);
        }
        bool x_336 = (d >= 0);
        x_342 = x_336;
        if (x_336) {
          x_341 = (p.x < 14);
          x_342 = x_341;
        }
        x_355 = x_342;
        if (x_342) {
          x_354 = (map[((p.x + 2) + (p.y * 16))] == 0);
          x_355 = x_354;
        }
        if (x_355) {
          d = (d - 1);
          int x_361 = p.x;
          int x_363 = p.y;
          map[(x_361 + (x_363 * 16))] = 1;
          int x_368 = p.x;
          int x_371 = p.y;
          map[((x_368 + 1) + (x_371 * 16))] = 1;
          int x_376 = p.x;
          int x_379 = p.y;
          map[((x_376 + 2) + (x_379 * 16))] = 1;
          p[0u] = (p.x + 2);
        }
        bool x_388 = (d >= 0);
        x_394 = x_388;
        if (x_388) {
          x_393 = (p.y < 14);
          x_394 = x_393;
        }
        x_407 = x_394;
        if (x_394) {
          x_406 = (map[(p.x + ((p.y + 2) * 16))] == 0);
          x_407 = x_406;
        }
        if (x_407) {
          d = (d - 1);
          int x_413 = p.x;
          int x_415 = p.y;
          map[(x_413 + (x_415 * 16))] = 1;
          int x_420 = p.x;
          int x_422 = p.y;
          map[(x_420 + ((x_422 + 1) * 16))] = 1;
          int x_428 = p.x;
          int x_430 = p.y;
          map[(x_428 + ((x_430 + 2) * 16))] = 1;
          p[1u] = (p.y + 2);
        }
      }
      if ((map[((ipos.y * 16) + ipos.x)] == 1)) {
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
