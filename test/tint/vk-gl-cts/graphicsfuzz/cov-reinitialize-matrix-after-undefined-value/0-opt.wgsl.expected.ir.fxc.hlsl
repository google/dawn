SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_5 : register(b0) {
  uint4 x_5[4];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float2x2 m = float2x2((0.0f).xx, (0.0f).xx);
  float f = 0.0f;
  int i = 0;
  int j = 0;
  int x_36 = asint(x_5[1u].x);
  if ((x_36 == 1)) {
    float x_40 = f;
    float2 v = float2(x_40, 0.0f);
    m = float2x2(v, float2(0.0f, x_40));
  }
  int x_45 = asint(x_5[1u].x);
  i = x_45;
  {
    while(true) {
      int x_50 = i;
      int x_52 = asint(x_5[0u].x);
      if ((x_50 < x_52)) {
      } else {
        break;
      }
      int x_56 = asint(x_5[1u].x);
      j = x_56;
      {
        while(true) {
          int x_61 = j;
          int x_63 = asint(x_5[0u].x);
          if ((x_61 < x_63)) {
          } else {
            break;
          }
          int x_66 = i;
          int x_67 = j;
          int x_68 = i;
          int x_70 = asint(x_5[0u].x);
          int x_72 = j;
          m[x_66][x_67] = float(((x_68 * x_70) + x_72));
          {
            int x_76 = j;
            j = (x_76 + 1);
          }
          continue;
        }
      }
      {
        int x_78 = i;
        i = (x_78 + 1);
      }
      continue;
    }
  }
  float2x2 x_80 = m;
  int x_82 = asint(x_5[1u].x);
  int x_85 = asint(x_5[2u].x);
  int x_88 = asint(x_5[0u].x);
  int x_91 = asint(x_5[3u].x);
  float v_1 = float(x_82);
  float2 v_2 = float2(v_1, float(x_85));
  float v_3 = float(x_88);
  float2x2 x_95 = float2x2(v_2, float2(v_3, float(x_91)));
  bool v_4 = all((x_80[0u] == x_95[0u]));
  if ((v_4 & all((x_80[1u] == x_95[1u])))) {
    int x_109 = asint(x_5[2u].x);
    int x_112 = asint(x_5[1u].x);
    int x_115 = asint(x_5[1u].x);
    int x_118 = asint(x_5[2u].x);
    float v_5 = float(x_109);
    float v_6 = float(x_112);
    float v_7 = float(x_115);
    x_GLF_color = float4(v_5, v_6, v_7, float(x_118));
  } else {
    int x_122 = asint(x_5[1u].x);
    float x_123 = float(x_122);
    x_GLF_color = float4(x_123, x_123, x_123, x_123);
  }
}

main_out main_inner() {
  main_1();
  main_out v_8 = {x_GLF_color};
  return v_8;
}

main_outputs main() {
  main_out v_9 = main_inner();
  main_outputs v_10 = {v_9.x_GLF_color_1};
  return v_10;
}

FXC validation failure:
<scrubbed_path>(50,11-17): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(38,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(28,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
