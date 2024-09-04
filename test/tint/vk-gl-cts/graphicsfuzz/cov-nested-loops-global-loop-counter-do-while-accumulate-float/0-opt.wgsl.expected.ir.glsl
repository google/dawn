SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


int x_GLF_global_loop_count = 0;
uniform buf0 x_7;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f = 0.0f;
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  int i_3 = 0;
  int i_4 = 0;
  int i_5 = 0;
  int i_6 = 0;
  int i_7 = 0;
  int i_8 = 0;
  int i_9 = 0;
  int i_10 = 0;
  int i_11 = 0;
  int i_12 = 0;
  int i_13 = 0;
  int i_14 = 0;
  float sum = 0.0f;
  int r = 0;
  x_GLF_global_loop_count = 0;
  float x_53 = x_7.x_GLF_uniform_float_values[1].el;
  f = x_53;
  int x_55 = x_10.x_GLF_uniform_int_values[1].el;
  i = x_55;
  {
    while(true) {
      int x_60 = i;
      int x_62 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_60 < x_62)) {
      } else {
        break;
      }
      int x_66 = x_10.x_GLF_uniform_int_values[1].el;
      i_1 = x_66;
      {
        while(true) {
          int x_71 = i_1;
          int x_73 = x_10.x_GLF_uniform_int_values[0].el;
          if ((x_71 < x_73)) {
          } else {
            break;
          }
          int x_77 = x_10.x_GLF_uniform_int_values[1].el;
          i_2 = x_77;
          {
            while(true) {
              int x_82 = i_2;
              int x_84 = x_10.x_GLF_uniform_int_values[0].el;
              if ((x_82 < x_84)) {
              } else {
                break;
              }
              int x_88 = x_10.x_GLF_uniform_int_values[1].el;
              i_3 = x_88;
              {
                while(true) {
                  int x_93 = i_3;
                  int x_95 = x_10.x_GLF_uniform_int_values[0].el;
                  if ((x_93 < x_95)) {
                  } else {
                    break;
                  }
                  int x_99 = x_10.x_GLF_uniform_int_values[1].el;
                  i_4 = x_99;
                  {
                    while(true) {
                      int x_104 = i_4;
                      int x_106 = x_10.x_GLF_uniform_int_values[0].el;
                      if ((x_104 < x_106)) {
                      } else {
                        break;
                      }
                      int x_110 = x_10.x_GLF_uniform_int_values[1].el;
                      i_5 = x_110;
                      {
                        while(true) {
                          int x_115 = i_5;
                          int x_117 = x_10.x_GLF_uniform_int_values[0].el;
                          if ((x_115 < x_117)) {
                          } else {
                            break;
                          }
                          int x_121 = x_10.x_GLF_uniform_int_values[1].el;
                          i_6 = x_121;
                          {
                            while(true) {
                              int x_126 = i_6;
                              int x_128 = x_10.x_GLF_uniform_int_values[0].el;
                              if ((x_126 < x_128)) {
                              } else {
                                break;
                              }
                              int x_132 = x_10.x_GLF_uniform_int_values[1].el;
                              i_7 = x_132;
                              {
                                while(true) {
                                  int x_137 = i_7;
                                  int x_139 = x_10.x_GLF_uniform_int_values[0].el;
                                  if ((x_137 < x_139)) {
                                  } else {
                                    break;
                                  }
                                  int x_143 = x_10.x_GLF_uniform_int_values[1].el;
                                  i_8 = x_143;
                                  {
                                    while(true) {
                                      int x_148 = i_8;
                                      int x_150 = x_10.x_GLF_uniform_int_values[0].el;
                                      if ((x_148 < x_150)) {
                                      } else {
                                        break;
                                      }
                                      int x_154 = x_10.x_GLF_uniform_int_values[1].el;
                                      i_9 = x_154;
                                      {
                                        while(true) {
                                          int x_159 = i_9;
                                          int x_161 = x_10.x_GLF_uniform_int_values[0].el;
                                          if ((x_159 < x_161)) {
                                          } else {
                                            break;
                                          }
                                          int x_165 = x_10.x_GLF_uniform_int_values[1].el;
                                          i_10 = x_165;
                                          {
                                            while(true) {
                                              int x_170 = i_10;
                                              int x_172 = x_10.x_GLF_uniform_int_values[0].el;
                                              if ((x_170 < x_172)) {
                                              } else {
                                                break;
                                              }
                                              int x_176 = x_10.x_GLF_uniform_int_values[1].el;
                                              i_11 = x_176;
                                              {
                                                while(true) {
                                                  int x_181 = i_11;
                                                  int x_183 = x_10.x_GLF_uniform_int_values[2].el;
                                                  if ((x_181 < x_183)) {
                                                  } else {
                                                    break;
                                                  }
                                                  int x_187 = x_10.x_GLF_uniform_int_values[1].el;
                                                  i_12 = x_187;
                                                  {
                                                    while(true) {
                                                      int x_192 = i_12;
                                                      int x_194 = x_10.x_GLF_uniform_int_values[0].el;
                                                      if ((x_192 < x_194)) {
                                                      } else {
                                                        break;
                                                      }
                                                      int x_198 = x_10.x_GLF_uniform_int_values[1].el;
                                                      i_13 = x_198;
                                                      {
                                                        while(true) {
                                                          int x_203 = i_13;
                                                          int x_205 = x_10.x_GLF_uniform_int_values[0].el;
                                                          if ((x_203 < x_205)) {
                                                          } else {
                                                            break;
                                                          }
                                                          int x_209 = x_10.x_GLF_uniform_int_values[1].el;
                                                          i_14 = x_209;
                                                          {
                                                            while(true) {
                                                              int x_214 = i_14;
                                                              int x_216 = x_10.x_GLF_uniform_int_values[2].el;
                                                              if ((x_214 < x_216)) {
                                                              } else {
                                                                break;
                                                              }
                                                              {
                                                                while(true) {
                                                                  int x_223 = x_GLF_global_loop_count;
                                                                  x_GLF_global_loop_count = (x_223 + 1);
                                                                  {
                                                                    int x_225 = x_GLF_global_loop_count;
                                                                    int x_227 = x_10.x_GLF_uniform_int_values[3].el;
                                                                    if (!((x_225 < (100 - x_227)))) { break; }
                                                                  }
                                                                  continue;
                                                                }
                                                              }
                                                              float x_231 = x_7.x_GLF_uniform_float_values[0].el;
                                                              float x_232 = f;
                                                              f = (x_232 + x_231);
                                                              {
                                                                int x_234 = i_14;
                                                                i_14 = (x_234 + 1);
                                                              }
                                                              continue;
                                                            }
                                                          }
                                                          {
                                                            int x_236 = i_13;
                                                            i_13 = (x_236 + 1);
                                                          }
                                                          continue;
                                                        }
                                                      }
                                                      {
                                                        int x_238 = i_12;
                                                        i_12 = (x_238 + 1);
                                                      }
                                                      continue;
                                                    }
                                                  }
                                                  {
                                                    int x_240 = i_11;
                                                    i_11 = (x_240 + 1);
                                                  }
                                                  continue;
                                                }
                                              }
                                              {
                                                int x_242 = i_10;
                                                i_10 = (x_242 + 1);
                                              }
                                              continue;
                                            }
                                          }
                                          {
                                            int x_244 = i_9;
                                            i_9 = (x_244 + 1);
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        int x_246 = i_8;
                                        i_8 = (x_246 + 1);
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    int x_248 = i_7;
                                    i_7 = (x_248 + 1);
                                  }
                                  continue;
                                }
                              }
                              {
                                int x_250 = i_6;
                                i_6 = (x_250 + 1);
                              }
                              continue;
                            }
                          }
                          {
                            int x_252 = i_5;
                            i_5 = (x_252 + 1);
                          }
                          continue;
                        }
                      }
                      {
                        int x_254 = i_4;
                        i_4 = (x_254 + 1);
                      }
                      continue;
                    }
                  }
                  {
                    int x_256 = i_3;
                    i_3 = (x_256 + 1);
                  }
                  continue;
                }
              }
              {
                int x_258 = i_2;
                i_2 = (x_258 + 1);
              }
              continue;
            }
          }
          {
            int x_260 = i_1;
            i_1 = (x_260 + 1);
          }
          continue;
        }
      }
      {
        int x_262 = i;
        i = (x_262 + 1);
      }
      continue;
    }
  }
  float x_265 = x_7.x_GLF_uniform_float_values[1].el;
  sum = x_265;
  int x_267 = x_10.x_GLF_uniform_int_values[1].el;
  r = x_267;
  {
    while(true) {
      int x_272 = x_GLF_global_loop_count;
      if ((x_272 < 100)) {
      } else {
        break;
      }
      int x_275 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_275 + 1);
      float x_277 = f;
      float x_278 = sum;
      sum = (x_278 + x_277);
      {
        int x_280 = r;
        r = (x_280 + 1);
      }
      continue;
    }
  }
  float x_282 = sum;
  float x_284 = x_7.x_GLF_uniform_float_values[2].el;
  if ((x_282 == x_284)) {
    int x_290 = x_10.x_GLF_uniform_int_values[0].el;
    int x_293 = x_10.x_GLF_uniform_int_values[1].el;
    int x_296 = x_10.x_GLF_uniform_int_values[1].el;
    int x_299 = x_10.x_GLF_uniform_int_values[0].el;
    float v = float(x_290);
    float v_1 = float(x_293);
    float v_2 = float(x_296);
    x_GLF_color = vec4(v, v_1, v_2, float(x_299));
  } else {
    int x_303 = x_10.x_GLF_uniform_int_values[1].el;
    float x_304 = float(x_303);
    x_GLF_color = vec4(x_304, x_304, x_304, x_304);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
