SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_6 : register(b0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_10 : register(b1) {
  uint4 x_10[4];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float4 v0 = (0.0f).xxxx;
  float4 v1 = (0.0f).xxxx;
  int a = 0;
  int c = 0;
  float x_41 = asfloat(x_6[1u].x);
  v0 = float4(x_41, x_41, x_41, x_41);
  float x_44 = asfloat(x_6[0u].x);
  v1 = float4(x_44, x_44, x_44, x_44);
  int x_47 = asint(x_10[1u].x);
  a = x_47;
  {
    while(true) {
      int x_52 = a;
      int x_54 = asint(x_10[0u].x);
      if ((x_52 < x_54)) {
      } else {
        break;
      }
      int x_58 = asint(x_10[3u].x);
      c = x_58;
      {
        while(true) {
          int x_63 = c;
          int x_65 = asint(x_10[2u].x);
          if ((x_63 < x_65)) {
          } else {
            break;
          }
          int x_68 = c;
          int x_69 = min(max(x_68, 0), 3);
          float x_71 = asfloat(x_6[1u].x);
          float x_73 = v0[x_69];
          v0[x_69] = (x_73 - x_71);
          int x_77 = asint(x_10[1u].x);
          int x_79 = asint(x_10[3u].x);
          if ((x_77 == x_79)) {
            int x_83 = a;
            float x_85 = asfloat(x_6[1u].x);
            float x_87 = asfloat(x_6[1u].x);
            float x_89 = asfloat(x_6[1u].x);
            float4 x_91 = v0;
            int x_93 = a;
            v1[x_83] = smoothstep(float4(x_85, x_87, x_89, 3.0f), (1.0f).xxxx, x_91)[x_93];
          }
          {
            int x_96 = c;
            c = (x_96 + 1);
          }
          continue;
        }
      }
      {
        int x_98 = a;
        a = (x_98 + 1);
      }
      continue;
    }
  }
  float x_101 = v1.x;
  float x_103 = asfloat(x_6[0u].x);
  if ((x_101 == x_103)) {
    int x_109 = asint(x_10[1u].x);
    int x_112 = asint(x_10[3u].x);
    int x_115 = asint(x_10[3u].x);
    int x_118 = asint(x_10[1u].x);
    float v = float(x_109);
    float v_1 = float(x_112);
    float v_2 = float(x_115);
    x_GLF_color = float4(v, v_1, v_2, float(x_118));
  } else {
    int x_122 = asint(x_10[3u].x);
    float x_123 = float(x_122);
    x_GLF_color = float4(x_123, x_123, x_123, x_123);
  }
}

main_out main_inner() {
  main_1();
  main_out v_3 = {x_GLF_color};
  return v_3;
}

main_outputs main() {
  main_out v_4 = main_inner();
  main_outputs v_5 = {v_4.x_GLF_color_1};
  return v_5;
}

FXC validation failure:
<scrubbed_path>(50,11-18): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(39,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(29,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
