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
  if ((asint(x_5[1u].x) == 1)) {
    float2 v = float2(f, 0.0f);
    m = float2x2(v, float2(0.0f, f));
  }
  i = asint(x_5[1u].x);
  {
    while(true) {
      int v_1 = i;
      if ((v_1 < asint(x_5[0u].x))) {
      } else {
        break;
      }
      j = asint(x_5[1u].x);
      {
        while(true) {
          int v_2 = j;
          if ((v_2 < asint(x_5[0u].x))) {
          } else {
            break;
          }
          int x_66 = i;
          int x_67 = j;
          int v_3 = i;
          int v_4 = (v_3 * asint(x_5[0u].x));
          m[x_66][x_67] = float((v_4 + j));
          {
            j = (j + 1);
          }
          continue;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float v_5 = float(asint(x_5[1u].x));
  float2 v_6 = float2(v_5, float(asint(x_5[2u].x)));
  float v_7 = float(asint(x_5[0u].x));
  float2x2 x_95 = float2x2(v_6, float2(v_7, float(asint(x_5[3u].x))));
  bool v_8 = all((m[0u] == x_95[0u]));
  if ((v_8 & all((m[1u] == x_95[1u])))) {
    float v_9 = float(asint(x_5[2u].x));
    float v_10 = float(asint(x_5[1u].x));
    float v_11 = float(asint(x_5[1u].x));
    x_GLF_color = float4(v_9, v_10, v_11, float(asint(x_5[2u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_5[1u].x))).xxxx);
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
<scrubbed_path>(43,11-17): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(33,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(25,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
