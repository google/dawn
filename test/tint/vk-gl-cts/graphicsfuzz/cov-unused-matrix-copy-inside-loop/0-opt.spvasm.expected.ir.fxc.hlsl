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
  float x_41 = float(asint(x_6[1u].x));
  float4 v_1 = float4(x_41, 0.0f, 0.0f, 0.0f);
  float4 v_2 = float4(0.0f, x_41, 0.0f, 0.0f);
  float4 v_3 = float4(0.0f, 0.0f, x_41, 0.0f);
  m0 = float4x4(v_1, v_2, v_3, float4(0.0f, 0.0f, 0.0f, x_41));
  c = asint(x_6[2u].x);
  {
    while(true) {
      int v_4 = c;
      if ((v_4 < asint(x_6[0u].x))) {
      } else {
        break;
      }
      m1 = m0;
      int x_59 = c;
      int x_61 = asint(x_6[3u].x);
      int x_64 = asint(x_6[2u].x);
      float4 v_5 = m1[tint_mod_i32(x_59, x_61)];
      v_5[x_64] = asfloat(x_10[0u].x);
      int x_68 = c;
      int x_70 = asint(x_6[3u].x);
      int x_73 = asint(x_6[2u].x);
      float4 v_6 = m0[tint_mod_i32(x_68, x_70)];
      v_6[x_73] = asfloat(x_10[0u].x);
      {
        c = (c + 1);
      }
      continue;
    }
  }
  float v_7 = float(asint(x_6[1u].x));
  float v_8 = float(asint(x_6[2u].x));
  float v_9 = float(asint(x_6[1u].x));
  float4 v_10 = float4(v_7, v_8, v_9, float(asint(x_6[1u].x)));
  float v_11 = float(asint(x_6[1u].x));
  float v_12 = float(asint(x_6[2u].x));
  float v_13 = float(asint(x_6[1u].x));
  float4 v_14 = float4(v_11, v_12, v_13, float(asint(x_6[1u].x)));
  float v_15 = float(asint(x_6[1u].x));
  float v_16 = float(asint(x_6[2u].x));
  float v_17 = float(asint(x_6[1u].x));
  float4 v_18 = float4(v_15, v_16, v_17, float(asint(x_6[1u].x)));
  float v_19 = float(asint(x_6[1u].x));
  float v_20 = float(asint(x_6[2u].x));
  float v_21 = float(asint(x_6[1u].x));
  float4x4 x_132 = float4x4(v_10, v_14, v_18, float4(v_19, v_20, v_21, float(asint(x_6[1u].x))));
  bool v_22 = all((m0[0u] == x_132[0u]));
  bool v_23 = (v_22 & all((m0[1u] == x_132[1u])));
  bool v_24 = (v_23 & all((m0[2u] == x_132[2u])));
  if ((v_24 & all((m0[3u] == x_132[3u])))) {
    float v_25 = float(asint(x_6[2u].x));
    float v_26 = float(asint(x_6[1u].x));
    float v_27 = float(asint(x_6[1u].x));
    x_GLF_color = float4(v_25, v_26, v_27, float(asint(x_6[2u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_6[1u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_28 = {x_GLF_color};
  return v_28;
}

main_outputs main() {
  main_out v_29 = main_inner();
  main_outputs v_30 = {v_29.x_GLF_color_1};
  return v_30;
}

FXC validation failure:
<scrubbed_path>(19,19-25): warning X3556: integer divides may be much slower, try using uints if possible.
<scrubbed_path>(44,7-15): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(33,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
