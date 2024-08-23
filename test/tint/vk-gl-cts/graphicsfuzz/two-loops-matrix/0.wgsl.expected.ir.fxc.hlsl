SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_8 : register(b0) {
  uint4 x_8[4];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  int x = 0;
  float4 matrix_u = (0.0f).xxxx;
  int b = 0;
  float4 matrix_b = (0.0f).xxxx;
  float4 x_42 = (0.0f).xxxx;
  x = 4;
  {
    while(true) {
      int x_10 = x;
      if ((x_10 >= 1)) {
      } else {
        break;
      }
      int x_11 = x;
      matrix_u[x_11] = 1.0f;
      {
        int x_12 = x;
        x = (x_12 - 1);
      }
      continue;
    }
  }
  b = 4;
  {
    while(true) {
      float x_55 = asfloat(x_8[0u].x);
      if ((x_55 < -1.0f)) {
      } else {
        break;
      }
      int x_14 = b;
      int x_15 = b;
      if ((x_15 > 1)) {
        float4 x_62 = matrix_b;
        float4 x_63 = matrix_b;
        x_42 = min(x_62, x_63);
      } else {
        float4 x_65 = matrix_u;
        x_42 = x_65;
      }
      float x_67 = x_42.y;
      matrix_b[x_14] = x_67;
      {
        int x_16 = b;
        b = (x_16 - 1);
      }
      continue;
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
}

main_out main_inner() {
  main_1();
  main_out v = {x_GLF_color};
  return v;
}

main_outputs main() {
  main_out v_1 = main_inner();
  main_outputs v_2 = {v_1.x_GLF_color_1};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(29,7-20): error X3504: literal loop terminated early due to out of bounds array access
<scrubbed_path>(22,5-15): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll


tint executable returned error: exit status 1
