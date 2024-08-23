SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_6 : register(b1) {
  uint4 x_6[5];
};
cbuffer cbuffer_x_9 : register(b0) {
  uint4 x_9[2];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float4 v = (0.0f).xxxx;
  int i = 0;
  int x_40 = asint(x_6[1u].x);
  int x_43 = asint(x_6[2u].x);
  int x_46 = asint(x_6[3u].x);
  int x_49 = asint(x_6[0u].x);
  float v_1 = float(x_40);
  float v_2 = float(x_43);
  float v_3 = float(x_46);
  v = float4(v_1, v_2, v_3, float(x_49));
  int x_53 = asint(x_6[4u].x);
  i = x_53;
  {
    while(true) {
      int x_58 = i;
      int x_60 = asint(x_6[0u].x);
      if ((x_58 < x_60)) {
      } else {
        break;
      }
      float4 x_63 = v;
      float4 x_64 = v;
      float4 x_65 = v;
      float4 x_66 = v;
      int x_88 = i;
      float x_92 = asfloat(x_9[0u].x);
      float4 v_4 = float4(x_63[0u], x_63[1u], x_63[2u], x_63[3u]);
      float4 v_5 = float4(x_64[0u], x_64[1u], x_64[2u], x_64[3u]);
      float4 v_6 = float4(x_65[0u], x_65[1u], x_65[2u], x_65[3u]);
      if ((float4x4(v_4, v_5, v_6, float4(x_66[0u], x_66[1u], x_66[2u], x_66[3u]))[0u][x_88] > x_92)) {
        int x_96 = i;
        float4 x_97 = v;
        float x_99 = asfloat(x_9[1u].x);
        float x_102 = asfloat(x_9[0u].x);
        int x_106 = asint(x_6[1u].x);
        float4 v_7 = float4(x_99, x_99, x_99, x_99);
        v[x_96] = clamp(x_97, v_7, float4(x_102, x_102, x_102, x_102))[x_106];
      }
      {
        int x_109 = i;
        i = (x_109 + 1);
      }
      continue;
    }
  }
  float4 x_111 = v;
  int x_113 = asint(x_6[1u].x);
  float x_114 = float(x_113);
  if (all((x_111 == float4(x_114, x_114, x_114, x_114)))) {
    int x_122 = asint(x_6[1u].x);
    int x_125 = asint(x_6[4u].x);
    int x_128 = asint(x_6[4u].x);
    int x_131 = asint(x_6[1u].x);
    float v_8 = float(x_122);
    float v_9 = float(x_125);
    float v_10 = float(x_128);
    x_GLF_color = float4(v_8, v_9, v_10, float(x_131));
  } else {
    int x_135 = asint(x_6[4u].x);
    float x_136 = float(x_135);
    x_GLF_color = float4(x_136, x_136, x_136, x_136);
  }
}

main_out main_inner() {
  main_1();
  main_out v_11 = {x_GLF_color};
  return v_11;
}

main_outputs main() {
  main_out v_12 = main_inner();
  main_outputs v_13 = {v_12.x_GLF_color_1};
  return v_13;
}

FXC validation failure:
<scrubbed_path>(54,9-15): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(31,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
