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
  int x_36 = asint(x_6[3u].x);
  float x_37 = float(x_36);
  v = float4(x_37, x_37, x_37, x_37);
  int x_40 = asint(x_6[0u].x);
  i = x_40;
  {
    while(true) {
      int x_45 = i;
      int x_47 = asint(x_6[3u].x);
      if ((x_45 < x_47)) {
      } else {
        break;
      }
      int x_50 = i;
      int x_51 = i;
      v[uint3(0u, 1u, 2u)[x_50]] = float(x_51);
      {
        int x_55 = i;
        i = (x_55 + 1);
      }
      continue;
    }
  }
  float4 x_57 = v;
  int x_59 = asint(x_6[0u].x);
  int x_62 = asint(x_6[1u].x);
  int x_65 = asint(x_6[2u].x);
  int x_68 = asint(x_6[3u].x);
  float v_1 = float(x_59);
  float v_2 = float(x_62);
  float v_3 = float(x_65);
  if (all((x_57 == float4(v_1, v_2, v_3, float(x_68))))) {
    int x_77 = asint(x_6[1u].x);
    int x_80 = asint(x_6[0u].x);
    int x_83 = asint(x_6[0u].x);
    int x_86 = asint(x_6[1u].x);
    float v_4 = float(x_77);
    float v_5 = float(x_80);
    float v_6 = float(x_83);
    x_GLF_color = float4(v_4, v_5, v_6, float(x_86));
  } else {
    int x_90 = asint(x_6[0u].x);
    float x_91 = float(x_90);
    x_GLF_color = float4(x_91, x_91, x_91, x_91);
  }
}

main_out main_inner() {
  main_1();
  main_out v_7 = {x_GLF_color};
  return v_7;
}

main_outputs main() {
  main_out v_8 = main_inner();
  main_outputs v_9 = {v_8.x_GLF_color_1};
  return v_9;
}

FXC validation failure:
<scrubbed_path>(32,7-32): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(23,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
