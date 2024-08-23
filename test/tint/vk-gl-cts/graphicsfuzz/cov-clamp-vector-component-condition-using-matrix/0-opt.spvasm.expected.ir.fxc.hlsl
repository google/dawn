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
  float v_1 = float(asint(x_6[1u].x));
  float v_2 = float(asint(x_6[2u].x));
  float v_3 = float(asint(x_6[3u].x));
  v = float4(v_1, v_2, v_3, float(asint(x_6[0u].x)));
  i = asint(x_6[4u].x);
  {
    while(true) {
      int v_4 = i;
      if ((v_4 < asint(x_6[0u].x))) {
      } else {
        break;
      }
      float4 v_5 = float4(v.x, v.y, v.z, v.w);
      float4 v_6 = float4(v.x, v.y, v.z, v.w);
      float4 v_7 = float4(v.x, v.y, v.z, v.w);
      float4x4 v_8 = float4x4(v_5, v_6, v_7, float4(v.x, v.y, v.z, v.w));
      float v_9 = v_8[0u][i];
      if ((v_9 > asfloat(x_9[0u].x))) {
        int x_96 = i;
        float4 v_10 = v;
        float4 v_11 = float4((asfloat(x_9[1u].x)).xxxx);
        float4 v_12 = clamp(v_10, v_11, float4((asfloat(x_9[0u].x)).xxxx));
        v[x_96] = v_12[asint(x_6[1u].x)];
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float4 v_13 = v;
  if (all((v_13 == float4((float(asint(x_6[1u].x))).xxxx)))) {
    float v_14 = float(asint(x_6[1u].x));
    float v_15 = float(asint(x_6[4u].x));
    float v_16 = float(asint(x_6[4u].x));
    x_GLF_color = float4(v_14, v_15, v_16, float(asint(x_6[1u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_6[4u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_17 = {x_GLF_color};
  return v_17;
}

main_outputs main() {
  main_out v_18 = main_inner();
  main_outputs v_19 = {v_18.x_GLF_color_1};
  return v_19;
}

FXC validation failure:
<scrubbed_path>(42,9-15): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(26,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
