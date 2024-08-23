SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_6 : register(b0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float4 v = (0.0f).xxxx;
  int i = 0;
  v = float4((float(asint(x_6[3u].x))).xxxx);
  i = asint(x_6[0u].x);
  {
    while(true) {
      int v_1 = i;
      if ((v_1 < asint(x_6[3u].x))) {
      } else {
        break;
      }
      int x_50 = i;
      v[uint3(0u, 1u, 2u)[x_50]] = float(i);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float4 v_2 = v;
  float v_3 = float(asint(x_6[0u].x));
  float v_4 = float(asint(x_6[1u].x));
  float v_5 = float(asint(x_6[2u].x));
  if (all((v_2 == float4(v_3, v_4, v_5, float(asint(x_6[3u].x)))))) {
    float v_6 = float(asint(x_6[1u].x));
    float v_7 = float(asint(x_6[0u].x));
    float v_8 = float(asint(x_6[0u].x));
    x_GLF_color = float4(v_6, v_7, v_8, float(asint(x_6[1u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_6[0u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_9 = {x_GLF_color};
  return v_9;
}

main_outputs main() {
  main_out v_10 = main_inner();
  main_outputs v_11 = {v_10.x_GLF_color_1};
  return v_11;
}

FXC validation failure:
<scrubbed_path>(27,7-32): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(20,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
