SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_7 : register(b1) {
  uint4 x_7[18];
};
cbuffer cbuffer_x_12 : register(b0) {
  uint4 x_12[4];
};
cbuffer cbuffer_x_15 : register(b2) {
  uint4 x_15[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
static bool continue_execution = true;
void main_1() {
  float4x4 m = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4 v = (0.0f).xxxx;
  float f = 0.0f;
  int a = 0;
  int b = 0;
  float zero = 0.0f;
  x_GLF_global_loop_count = 0;
  float x_62 = asfloat(x_7[1u].x);
  float x_64 = asfloat(x_7[2u].x);
  float x_66 = asfloat(x_7[3u].x);
  float x_68 = asfloat(x_7[4u].x);
  float x_70 = asfloat(x_7[5u].x);
  float x_72 = asfloat(x_7[6u].x);
  float x_74 = asfloat(x_7[7u].x);
  float x_76 = asfloat(x_7[8u].x);
  float x_78 = asfloat(x_7[9u].x);
  float x_80 = asfloat(x_7[10u].x);
  float x_82 = asfloat(x_7[11u].x);
  float x_84 = asfloat(x_7[12u].x);
  float x_86 = asfloat(x_7[13u].x);
  float x_88 = asfloat(x_7[14u].x);
  float x_90 = asfloat(x_7[15u].x);
  float x_92 = asfloat(x_7[16u].x);
  float4 v_1 = float4(x_62, x_64, x_66, x_68);
  float4 v_2 = float4(x_70, x_72, x_74, x_76);
  float4 v_3 = float4(x_78, x_80, x_82, x_84);
  m = float4x4(v_1, v_2, v_3, float4(x_86, x_88, x_90, x_92));
  float x_99 = asfloat(x_7[1u].x);
  float x_101 = asfloat(x_7[2u].x);
  float x_103 = asfloat(x_7[3u].x);
  float x_105 = asfloat(x_7[4u].x);
  v = float4(x_99, x_101, x_103, x_105);
  float x_108 = asfloat(x_7[1u].x);
  f = x_108;
  int x_110 = asint(x_12[0u].x);
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
      float x_123 = asfloat(x_7[1u].x);
      float x_125 = v[x_121];
      v[x_121] = (x_125 + x_123);
      int x_129 = asint(x_12[2u].x);
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
      float x_156 = asfloat(x_7[1u].x);
      m[1][min(max(x_153, 0), 3)] = x_156;
      int x_159 = asint(x_15[0u].x);
      int x_161 = asint(x_12[0u].x);
      if ((x_159 == x_161)) {
        continue_execution = false;
      }
      int x_166 = asint(x_15[0u].x);
      int x_168 = asint(x_12[1u].x);
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
  float x_175 = asfloat(x_7[0u].x);
  zero = x_175;
  float x_176 = f;
  float x_178 = asfloat(x_7[17u].x);
  if (!((x_176 == x_178))) {
    float x_183 = asfloat(x_7[1u].x);
    zero = x_183;
  }
  float x_184 = f;
  float x_185 = zero;
  int x_187 = asint(x_12[0u].x);
  float x_189 = f;
  x_GLF_color = float4(x_184, x_185, float(x_187), x_189);
}

main_out main_inner() {
  main_1();
  main_out v_4 = {x_GLF_color};
  return v_4;
}

main_outputs main() {
  main_out v_5 = main_inner();
  main_outputs v_6 = {v_5.x_GLF_color_1};
  if (!(continue_execution)) {
    discard;
  }
  main_outputs v_7 = v_6;
  return v_7;
}

FXC validation failure:
<scrubbed_path>(72,7-14): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(60,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
