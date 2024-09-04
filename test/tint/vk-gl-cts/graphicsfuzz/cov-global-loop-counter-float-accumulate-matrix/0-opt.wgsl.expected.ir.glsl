SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[18];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct buf2 {
  int one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


int x_GLF_global_loop_count = 0;
uniform buf1 x_7;
uniform buf0 x_12;
uniform buf2 x_15;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  mat4 m = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 v = vec4(0.0f);
  float f = 0.0f;
  int a = 0;
  int b = 0;
  float zero = 0.0f;
  x_GLF_global_loop_count = 0;
  float x_62 = x_7.x_GLF_uniform_float_values[1].el;
  float x_64 = x_7.x_GLF_uniform_float_values[2].el;
  float x_66 = x_7.x_GLF_uniform_float_values[3].el;
  float x_68 = x_7.x_GLF_uniform_float_values[4].el;
  float x_70 = x_7.x_GLF_uniform_float_values[5].el;
  float x_72 = x_7.x_GLF_uniform_float_values[6].el;
  float x_74 = x_7.x_GLF_uniform_float_values[7].el;
  float x_76 = x_7.x_GLF_uniform_float_values[8].el;
  float x_78 = x_7.x_GLF_uniform_float_values[9].el;
  float x_80 = x_7.x_GLF_uniform_float_values[10].el;
  float x_82 = x_7.x_GLF_uniform_float_values[11].el;
  float x_84 = x_7.x_GLF_uniform_float_values[12].el;
  float x_86 = x_7.x_GLF_uniform_float_values[13].el;
  float x_88 = x_7.x_GLF_uniform_float_values[14].el;
  float x_90 = x_7.x_GLF_uniform_float_values[15].el;
  float x_92 = x_7.x_GLF_uniform_float_values[16].el;
  vec4 v_1 = vec4(x_62, x_64, x_66, x_68);
  vec4 v_2 = vec4(x_70, x_72, x_74, x_76);
  vec4 v_3 = vec4(x_78, x_80, x_82, x_84);
  m = mat4(v_1, v_2, v_3, vec4(x_86, x_88, x_90, x_92));
  float x_99 = x_7.x_GLF_uniform_float_values[1].el;
  float x_101 = x_7.x_GLF_uniform_float_values[2].el;
  float x_103 = x_7.x_GLF_uniform_float_values[3].el;
  float x_105 = x_7.x_GLF_uniform_float_values[4].el;
  v = vec4(x_99, x_101, x_103, x_105);
  float x_108 = x_7.x_GLF_uniform_float_values[1].el;
  f = x_108;
  int x_110 = x_12.x_GLF_uniform_int_values[0].el;
  a = x_110;
  {
    while(true) {
      int x_115 = x_GLF_global_loop_count;
      if ((x_115 < 10)) {
      } else {
        break;
      }
      int x_118 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_118 + 1);
      int x_120 = a;
      int x_121 = min(max(x_120, 0), 3);
      float x_123 = x_7.x_GLF_uniform_float_values[1].el;
      float x_125 = v[x_121];
      v[x_121] = (x_125 + x_123);
      int x_129 = x_12.x_GLF_uniform_int_values[2].el;
      b = x_129;
      {
        while(true) {
          int x_134 = x_GLF_global_loop_count;
          if ((x_134 < 10)) {
          } else {
            break;
          }
          int x_137 = x_GLF_global_loop_count;
          x_GLF_global_loop_count = (x_137 + 1);
          int x_139 = b;
          float x_142 = v[min(max(x_139, 0), 3)];
          int x_143 = b;
          int x_145 = a;
          float x_147 = m[min(max(x_143, 0), 3)][x_145];
          float x_149 = f;
          f = (x_149 + (x_142 * x_147));
          {
            int x_151 = b;
            b = (x_151 - 1);
          }
          continue;
        }
      }
      int x_153 = a;
      float x_156 = x_7.x_GLF_uniform_float_values[1].el;
      m[1][min(max(x_153, 0), 3)] = x_156;
      int x_159 = x_15.one;
      int x_161 = x_12.x_GLF_uniform_int_values[0].el;
      if ((x_159 == x_161)) {
        continue_execution = false;
      }
      int x_166 = x_15.one;
      int x_168 = x_12.x_GLF_uniform_int_values[1].el;
      if ((x_166 == x_168)) {
        continue_execution = false;
      }
      {
        int x_172 = a;
        a = (x_172 + 1);
      }
      continue;
    }
  }
  float x_175 = x_7.x_GLF_uniform_float_values[0].el;
  zero = x_175;
  float x_176 = f;
  float x_178 = x_7.x_GLF_uniform_float_values[17].el;
  if (!((x_176 == x_178))) {
    float x_183 = x_7.x_GLF_uniform_float_values[1].el;
    zero = x_183;
  }
  float x_184 = f;
  float x_185 = zero;
  int x_187 = x_12.x_GLF_uniform_int_values[0].el;
  float x_189 = f;
  x_GLF_color = vec4(x_184, x_185, float(x_187), x_189);
}
main_out main() {
  main_1();
  main_out v_4 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_4;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
