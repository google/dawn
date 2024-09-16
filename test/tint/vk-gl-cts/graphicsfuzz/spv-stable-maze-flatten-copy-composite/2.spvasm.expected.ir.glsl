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
      int x_80 = i;
      map[x_80] = 0;
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
      bool x_104 = false;
      bool x_105 = false;
      bool x_124 = false;
      bool x_125 = false;
      bool x_144 = false;
      bool x_145 = false;
      bool x_164 = false;
      bool x_165 = false;
      v = (v + 1);
      directions = 0;
      bool x_92 = (p.x > 0);
      x_105 = x_92;
      if (x_92) {
        x_104 = (map[((p.x - 2) + (p.y * 16))] == 0);
        x_105 = x_104;
      }
      if (x_105) {
        directions = (directions + 1);
      }
      bool x_112 = (p.y > 0);
      x_125 = x_112;
      if (x_112) {
        x_124 = (map[(p.x + ((p.y - 2) * 16))] == 0);
        x_125 = x_124;
      }
      if (x_125) {
        directions = (directions + 1);
      }
      bool x_132 = (p.x < 14);
      x_145 = x_132;
      if (x_132) {
        x_144 = (map[((p.x + 2) + (p.y * 16))] == 0);
        x_145 = x_144;
      }
      if (x_145) {
        directions = (directions + 1);
      }
      bool x_152 = (p.y < 14);
      x_165 = x_152;
      if (x_152) {
        x_164 = (map[(p.x + ((p.y + 2) * 16))] == 0);
        x_165 = x_164;
      }
      if (x_165) {
        directions = (directions + 1);
      }
      bool x_229 = false;
      bool x_230 = false;
      bool x_242 = false;
      bool x_243 = false;
      bool x_281 = false;
      bool x_282 = false;
      bool x_295 = false;
      bool x_296 = false;
      bool x_335 = false;
      bool x_336 = false;
      bool x_348 = false;
      bool x_349 = false;
      bool x_387 = false;
      bool x_388 = false;
      bool x_400 = false;
      bool x_401 = false;
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
        int x_211 = p.x;
        int x_213 = p.y;
        map[(x_211 + (x_213 * 16))] = 1;
      } else {
        d = tint_mod_i32(v, directions);
        v = (v + directions);
        bool x_224 = (d >= 0);
        x_230 = x_224;
        if (x_224) {
          x_229 = (p.x > 0);
          x_230 = x_229;
        }
        x_243 = x_230;
        if (x_230) {
          x_242 = (map[((p.x - 2) + (p.y * 16))] == 0);
          x_243 = x_242;
        }
        if (x_243) {
          d = (d - 1);
          int x_249 = p.x;
          int x_251 = p.y;
          map[(x_249 + (x_251 * 16))] = 1;
          int x_256 = p.x;
          int x_259 = p.y;
          map[((x_256 - 1) + (x_259 * 16))] = 1;
          int x_264 = p.x;
          int x_267 = p.y;
          map[((x_264 - 2) + (x_267 * 16))] = 1;
          p[0u] = (p.x - 2);
        }
        bool x_276 = (d >= 0);
        x_282 = x_276;
        if (x_276) {
          x_281 = (p.y > 0);
          x_282 = x_281;
        }
        x_296 = x_282;
        if (x_282) {
          int x_286 = p.x;
          int x_288 = p.y;
          int x_291[256] = map;
          map = int[256](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          map = x_291;
          x_295 = (map[(x_286 + ((x_288 - 2) * 16))] == 0);
          x_296 = x_295;
        }
        if (x_296) {
          d = (d - 1);
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
          p[1u] = (p.y - 2);
        }
        bool x_330 = (d >= 0);
        x_336 = x_330;
        if (x_330) {
          x_335 = (p.x < 14);
          x_336 = x_335;
        }
        x_349 = x_336;
        if (x_336) {
          x_348 = (map[((p.x + 2) + (p.y * 16))] == 0);
          x_349 = x_348;
        }
        if (x_349) {
          d = (d - 1);
          int x_355 = p.x;
          int x_357 = p.y;
          map[(x_355 + (x_357 * 16))] = 1;
          int x_362 = p.x;
          int x_365 = p.y;
          map[((x_362 + 1) + (x_365 * 16))] = 1;
          int x_370 = p.x;
          int x_373 = p.y;
          map[((x_370 + 2) + (x_373 * 16))] = 1;
          p[0u] = (p.x + 2);
        }
        bool x_382 = (d >= 0);
        x_388 = x_382;
        if (x_382) {
          x_387 = (p.y < 14);
          x_388 = x_387;
        }
        x_401 = x_388;
        if (x_388) {
          x_400 = (map[(p.x + ((p.y + 2) * 16))] == 0);
          x_401 = x_400;
        }
        if (x_401) {
          d = (d - 1);
          int x_407 = p.x;
          int x_409 = p.y;
          map[(x_407 + (x_409 * 16))] = 1;
          int x_414 = p.x;
          int x_416 = p.y;
          map[(x_414 + ((x_416 + 1) * 16))] = 1;
          int x_422 = p.x;
          int x_424 = p.y;
          map[(x_422 + ((x_424 + 2) * 16))] = 1;
          p[1u] = (p.y + 2);
        }
      }
      if ((map[((ipos.y * 16) + ipos.x)] == 1)) {
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
