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
      int x_78 = i;
      map[x_78] = 0;
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
      bool x_102 = false;
      bool x_103 = false;
      bool x_122 = false;
      bool x_123 = false;
      bool x_142 = false;
      bool x_143 = false;
      bool x_162 = false;
      bool x_163 = false;
      v = (v + 1);
      directions = 0;
      bool x_90 = (p.x > 0);
      x_103 = x_90;
      if (x_90) {
        x_102 = (map[((p.x - 2) + (p.y * 16))] == 0);
        x_103 = x_102;
      }
      if (x_103) {
        directions = (directions + 1);
      }
      bool x_110 = (p.y > 0);
      x_123 = x_110;
      if (x_110) {
        x_122 = (map[(p.x + ((p.y - 2) * 16))] == 0);
        x_123 = x_122;
      }
      if (x_123) {
        directions = (directions + 1);
      }
      bool x_130 = (p.x < 14);
      x_143 = x_130;
      if (x_130) {
        x_142 = (map[((p.x + 2) + (p.y * 16))] == 0);
        x_143 = x_142;
      }
      if (x_143) {
        directions = (directions + 1);
      }
      bool x_150 = (p.y < 14);
      x_163 = x_150;
      if (x_150) {
        x_162 = (map[(p.x + ((p.y + 2) * 16))] == 0);
        x_163 = x_162;
      }
      if (x_163) {
        directions = (directions + 1);
      }
      bool x_227 = false;
      bool x_228 = false;
      bool x_240 = false;
      bool x_241 = false;
      bool x_279 = false;
      bool x_280 = false;
      bool x_292 = false;
      bool x_293 = false;
      bool x_331 = false;
      bool x_332 = false;
      bool x_344 = false;
      bool x_345 = false;
      bool x_383 = false;
      bool x_384 = false;
      bool x_396 = false;
      bool x_397 = false;
      if ((directions == 0)) {
        canwalk = false;
        i = 0;
        {
          while(true) {
            if ((i < 8)) {
            } else {
              break;
            }
            j = 0;
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
        int x_209 = p.x;
        int x_211 = p.y;
        map[(x_209 + (x_211 * 16))] = 1;
      } else {
        d = tint_mod_i32(v, directions);
        v = (v + directions);
        bool x_222 = (d >= 0);
        x_228 = x_222;
        if (x_222) {
          x_227 = (p.x > 0);
          x_228 = x_227;
        }
        x_241 = x_228;
        if (x_228) {
          x_240 = (map[((p.x - 2) + (p.y * 16))] == 0);
          x_241 = x_240;
        }
        if (x_241) {
          d = (d - 1);
          int x_247 = p.x;
          int x_249 = p.y;
          map[(x_247 + (x_249 * 16))] = 1;
          int x_254 = p.x;
          int x_257 = p.y;
          map[((x_254 - 1) + (x_257 * 16))] = 1;
          int x_262 = p.x;
          int x_265 = p.y;
          map[((x_262 - 2) + (x_265 * 16))] = 1;
          p[0u] = (p.x - 2);
        }
        bool x_274 = (d >= 0);
        x_280 = x_274;
        if (x_274) {
          x_279 = (p.y > 0);
          x_280 = x_279;
        }
        x_293 = x_280;
        if (x_280) {
          x_292 = (map[(p.x + ((p.y - 2) * 16))] == 0);
          x_293 = x_292;
        }
        if (x_293) {
          d = (d - 1);
          int x_299 = p.x;
          int x_301 = p.y;
          map[(x_299 + (x_301 * 16))] = 1;
          int x_306 = p.x;
          int x_308 = p.y;
          map[(x_306 + ((x_308 - 1) * 16))] = 1;
          int x_314 = p.x;
          int x_316 = p.y;
          map[(x_314 + ((x_316 - 2) * 16))] = 1;
          p[1u] = (p.y - 2);
        }
        bool x_326 = (d >= 0);
        x_332 = x_326;
        if (x_326) {
          x_331 = (p.x < 14);
          x_332 = x_331;
        }
        x_345 = x_332;
        if (x_332) {
          x_344 = (map[((p.x + 2) + (p.y * 16))] == 0);
          x_345 = x_344;
        }
        if (x_345) {
          d = (d - 1);
          int x_351 = p.x;
          int x_353 = p.y;
          map[(x_351 + (x_353 * 16))] = 1;
          int x_358 = p.x;
          int x_361 = p.y;
          map[((x_358 + 1) + (x_361 * 16))] = 1;
          int x_366 = p.x;
          int x_369 = p.y;
          map[((x_366 + 2) + (x_369 * 16))] = 1;
          p[0u] = (p.x + 2);
        }
        bool x_378 = (d >= 0);
        x_384 = x_378;
        if (x_378) {
          x_383 = (p.y < 14);
          x_384 = x_383;
        }
        x_397 = x_384;
        if (x_384) {
          x_396 = (map[(p.x + ((p.y + 2) * 16))] == 0);
          x_397 = x_396;
        }
        if (x_397) {
          d = (d - 1);
          int x_403 = p.x;
          int x_405 = p.y;
          map[(x_403 + (x_405 * 16))] = 1;
          int x_410 = p.x;
          int x_412 = p.y;
          map[(x_410 + ((x_412 + 1) * 16))] = 1;
          int x_418 = p.x;
          int x_420 = p.y;
          map[(x_418 + ((x_420 + 2) * 16))] = 1;
          p[1u] = (p.y + 2);
        }
      }
      if ((map[((ipos.y * 16) + ipos.x)] == 1)) {
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
