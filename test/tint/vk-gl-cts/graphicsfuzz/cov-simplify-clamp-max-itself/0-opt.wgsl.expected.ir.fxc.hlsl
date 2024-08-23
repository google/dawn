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
      int x_40 = i;
      int x_42 = asint(x_7[0u].w);
      if ((x_40 < (x_42 + 1))) {
      } else {
        break;
      }
      int x_46 = i;
      int x_48 = asint(x_7[0u].x);
      int x_49 = i;
      uint v = (uint(min(max(x_46, x_48), x_49)) * 4u);
      int x_52 = asint(x_7[(v / 16u)][((v % 16u) / 4u)]);
      if ((x_52 == 1)) {
        int x_57 = i;
        a[x_57] = 5;
      } else {
        int x_59 = i;
        int x_60 = i;
        a[x_59] = x_60;
      }
      {
        int x_62 = i;
        i = (x_62 + 1);
      }
      continue;
    }
  }
  int x_65 = a.x;
  int x_67 = a.y;
  int x_70 = a.z;
  int x_73 = a.w;
  sum = (((x_65 + x_67) + x_70) + x_73);
  int x_75 = sum;
  if ((x_75 == 10)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = (0.0f).xxxx;
  }
}

main_out main_inner() {
  main_1();
  main_out v_1 = {x_GLF_color};
  return v_1;
}

main_outputs main() {
  main_out v_2 = main_inner();
  main_outputs v_3 = {v_2.x_GLF_color_1};
  return v_3;
}

FXC validation failure:
<scrubbed_path>(21,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
