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
  v0 = float4((asfloat(x_6[1u].x)).xxxx);
  v1 = float4((asfloat(x_6[0u].x)).xxxx);
  a = asint(x_10[1u].x);
  {
    while(true) {
      int v = a;
      if ((v < asint(x_10[0u].x))) {
      } else {
        break;
      }
      c = asint(x_10[3u].x);
      {
        while(true) {
          int v_1 = c;
          if ((v_1 < asint(x_10[2u].x))) {
          } else {
            break;
          }
          int x_69 = min(max(c, 0), 3);
          float v_2 = v0[x_69];
          v0[x_69] = (v_2 - asfloat(x_6[1u].x));
          int v_3 = asint(x_10[1u].x);
          if ((v_3 == asint(x_10[3u].x))) {
            int x_83 = a;
            float v_4 = asfloat(x_6[1u].x);
            float v_5 = asfloat(x_6[1u].x);
            float4 v_6 = float4(v_4, v_5, asfloat(x_6[1u].x), 3.0f);
            float4 v_7 = smoothstep(v_6, (1.0f).xxxx, v0);
            v1[x_83] = v_7[a];
          }
          {
            c = (c + 1);
          }
          continue;
        }
      }
      {
        a = (a + 1);
      }
      continue;
    }
  }
  float v_8 = v1.x;
  if ((v_8 == asfloat(x_6[0u].x))) {
    float v_9 = float(asint(x_10[1u].x));
    float v_10 = float(asint(x_10[3u].x));
    float v_11 = float(asint(x_10[3u].x));
    x_GLF_color = float4(v_9, v_10, v_11, float(asint(x_10[1u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_10[3u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_12 = {x_GLF_color};
  return v_12;
}

main_outputs main() {
  main_out v_13 = main_inner();
  main_outputs v_14 = {v_13.x_GLF_color_1};
  return v_14;
}

FXC validation failure:
<scrubbed_path>(42,11-18): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(34,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(26,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
