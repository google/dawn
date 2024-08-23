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
      if ((x >= 1)) {
      } else {
        break;
      }
      int x_11 = x;
      matrix_u[x_11] = 1.0f;
      {
        x = (x - 1);
      }
      continue;
    }
  }
  b = 4;
  {
    while(true) {
      if ((asfloat(x_8[0u].x) < -1.0f)) {
      } else {
        break;
      }
      int x_14 = b;
      if ((b > 1)) {
        x_42 = min(matrix_b, matrix_b);
      } else {
        x_42 = matrix_u;
      }
      matrix_b[x_14] = x_42.y;
      {
        b = (b - 1);
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
<scrubbed_path>(28,7-20): error X3504: literal loop terminated early due to out of bounds array access
<scrubbed_path>(22,5-15): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll


tint executable returned error: exit status 1
