SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_6 : register(b1) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_10 : register(b0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
int tint_mod_i32(int lhs, int rhs) {
  int v = ((((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v) * v));
}

void main_1() {
  float4x4 m0 = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  int c = 0;
  float4x4 m1 = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  int x_40 = asint(x_6[1u].x);
  float x_41 = float(x_40);
  float4 v_1 = float4(x_41, 0.0f, 0.0f, 0.0f);
  float4 v_2 = float4(0.0f, x_41, 0.0f, 0.0f);
  float4 v_3 = float4(0.0f, 0.0f, x_41, 0.0f);
  m0 = float4x4(v_1, v_2, v_3, float4(0.0f, 0.0f, 0.0f, x_41));
  int x_48 = asint(x_6[2u].x);
  c = x_48;
  {
    while(true) {
      int x_53 = c;
      int x_55 = asint(x_6[0u].x);
      if ((x_53 < x_55)) {
      } else {
        break;
      }
      float4x4 x_58 = m0;
      m1 = x_58;
      int x_59 = c;
      int x_61 = asint(x_6[3u].x);
      int x_64 = asint(x_6[2u].x);
      float x_66 = asfloat(x_10[0u].x);
      m1[tint_mod_i32(x_59, x_61)][x_64] = x_66;
      int x_68 = c;
      int x_70 = asint(x_6[3u].x);
      int x_73 = asint(x_6[2u].x);
      float x_75 = asfloat(x_10[0u].x);
      m0[tint_mod_i32(x_68, x_70)][x_73] = x_75;
      {
        int x_77 = c;
        c = (x_77 + 1);
      }
      continue;
    }
  }
  float4x4 x_79 = m0;
  int x_81 = asint(x_6[1u].x);
  int x_84 = asint(x_6[2u].x);
  int x_87 = asint(x_6[1u].x);
  int x_90 = asint(x_6[1u].x);
  int x_93 = asint(x_6[1u].x);
  int x_96 = asint(x_6[2u].x);
  int x_99 = asint(x_6[1u].x);
  int x_102 = asint(x_6[1u].x);
  int x_105 = asint(x_6[1u].x);
  int x_108 = asint(x_6[2u].x);
  int x_111 = asint(x_6[1u].x);
  int x_114 = asint(x_6[1u].x);
  int x_117 = asint(x_6[1u].x);
  int x_120 = asint(x_6[2u].x);
  int x_123 = asint(x_6[1u].x);
  int x_126 = asint(x_6[1u].x);
  float v_4 = float(x_81);
  float v_5 = float(x_84);
  float v_6 = float(x_87);
  float4 v_7 = float4(v_4, v_5, v_6, float(x_90));
  float v_8 = float(x_93);
  float v_9 = float(x_96);
  float v_10 = float(x_99);
  float4 v_11 = float4(v_8, v_9, v_10, float(x_102));
  float v_12 = float(x_105);
  float v_13 = float(x_108);
  float v_14 = float(x_111);
  float4 v_15 = float4(v_12, v_13, v_14, float(x_114));
  float v_16 = float(x_117);
  float v_17 = float(x_120);
  float v_18 = float(x_123);
  float4x4 x_132 = float4x4(v_7, v_11, v_15, float4(v_16, v_17, v_18, float(x_126)));
  bool v_19 = all((x_79[0u] == x_132[0u]));
  bool v_20 = (v_19 & all((x_79[1u] == x_132[1u])));
  bool v_21 = (v_20 & all((x_79[2u] == x_132[2u])));
  if ((v_21 & all((x_79[3u] == x_132[3u])))) {
    int x_156 = asint(x_6[2u].x);
    int x_159 = asint(x_6[1u].x);
    int x_162 = asint(x_6[1u].x);
    int x_165 = asint(x_6[2u].x);
    float v_22 = float(x_156);
    float v_23 = float(x_159);
    float v_24 = float(x_162);
    x_GLF_color = float4(v_22, v_23, v_24, float(x_165));
  } else {
    int x_169 = asint(x_6[1u].x);
    float x_170 = float(x_169);
    x_GLF_color = float4(x_170, x_170, x_170, x_170);
  }
}

main_out main_inner() {
  main_1();
  main_out v_25 = {x_GLF_color};
  return v_25;
}

main_outputs main() {
  main_out v_26 = main_inner();
  main_outputs v_27 = {v_26.x_GLF_color_1};
  return v_27;
}

FXC validation failure:
<scrubbed_path>(19,19-25): warning X3556: integer divides may be much slower, try using uints if possible.
<scrubbed_path>(48,7-34): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(35,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
