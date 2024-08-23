SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  int4 a = (0).xxxx;
  int i = 0;
  int sum = 0;
  a = (0).xxxx;
  i = 0;
  {
    while(true) {
      int v = i;
      if ((v < (asint(x_7[0u].w) + 1))) {
      } else {
        break;
      }
      int v_1 = i;
      int v_2 = asint(x_7[0u].x);
      int v_3 = i;
      uint v_4 = (uint(min(max(v_1, v_2), v_3)) * 4u);
      if ((asint(x_7[(v_4 / 16u)][((v_4 % 16u) / 4u)]) == 1)) {
        int x_57 = i;
        a[x_57] = 5;
      } else {
        int x_59 = i;
        a[x_59] = i;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  sum = (((a.x + a.y) + a.z) + a.w);
  if ((sum == 10)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = (0.0f).xxxx;
  }
}

main_out main_inner() {
  main_1();
  main_out v_5 = {x_GLF_color};
  return v_5;
}

main_outputs main() {
  main_out v_6 = main_inner();
  main_outputs v_7 = {v_6.x_GLF_color_1};
  return v_7;
}

FXC validation failure:
<scrubbed_path>(21,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
