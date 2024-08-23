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
  float v_1 = asfloat(x_7[1u].x);
  float v_2 = asfloat(x_7[2u].x);
  float v_3 = asfloat(x_7[3u].x);
  float4 v_4 = float4(v_1, v_2, v_3, asfloat(x_7[4u].x));
  float v_5 = asfloat(x_7[5u].x);
  float v_6 = asfloat(x_7[6u].x);
  float v_7 = asfloat(x_7[7u].x);
  float4 v_8 = float4(v_5, v_6, v_7, asfloat(x_7[8u].x));
  float v_9 = asfloat(x_7[9u].x);
  float v_10 = asfloat(x_7[10u].x);
  float v_11 = asfloat(x_7[11u].x);
  float4 v_12 = float4(v_9, v_10, v_11, asfloat(x_7[12u].x));
  float v_13 = asfloat(x_7[13u].x);
  float v_14 = asfloat(x_7[14u].x);
  float v_15 = asfloat(x_7[15u].x);
  m = float4x4(v_4, v_8, v_12, float4(v_13, v_14, v_15, asfloat(x_7[16u].x)));
  float v_16 = asfloat(x_7[1u].x);
  float v_17 = asfloat(x_7[2u].x);
  float v_18 = asfloat(x_7[3u].x);
  v = float4(v_16, v_17, v_18, asfloat(x_7[4u].x));
  f = asfloat(x_7[1u].x);
  a = asint(x_12[0u].x);
  {
    while(true) {
      if ((x_GLF_global_loop_count < 10)) {
      } else {
        break;
      }
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      int x_121 = min(max(a, 0), 3);
      float v_19 = v[x_121];
      v[x_121] = (v_19 + asfloat(x_7[1u].x));
      b = asint(x_12[2u].x);
      {
        while(true) {
          if ((x_GLF_global_loop_count < 10)) {
          } else {
            break;
          }
          x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
          float v_20 = f;
          float v_21 = v[min(max(b, 0), 3)];
          float4 v_22 = m[min(max(b, 0), 3)];
          f = (v_20 + (v_21 * v_22[a]));
          {
            b = (b - 1);
          }
          continue;
        }
      }
      int x_153 = a;
      int v_23 = min(max(x_153, 0), 3);
      m[1][v_23] = asfloat(x_7[1u].x);
      int v_24 = asint(x_15[0u].x);
      if ((v_24 == asint(x_12[0u].x))) {
        continue_execution = false;
      }
      int v_25 = asint(x_15[0u].x);
      if ((v_25 == asint(x_12[1u].x))) {
        continue_execution = false;
      }
      {
        a = (a + 1);
      }
      continue;
    }
  }
  zero = asfloat(x_7[0u].x);
  float v_26 = f;
  if (!((v_26 == asfloat(x_7[17u].x)))) {
    zero = asfloat(x_7[1u].x);
  }
  float v_27 = f;
  float v_28 = zero;
  float v_29 = float(asint(x_12[0u].x));
  x_GLF_color = float4(v_27, v_28, v_29, f);
}

main_out main_inner() {
  main_1();
  main_out v_30 = {x_GLF_color};
  return v_30;
}

main_outputs main() {
  main_out v_31 = main_inner();
  main_outputs v_32 = {v_31.x_GLF_color_1};
  if (!(continue_execution)) {
    discard;
  }
  main_outputs v_33 = v_32;
  return v_33;
}

FXC validation failure:
<scrubbed_path>(61,7-14): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(53,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
