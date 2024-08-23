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
cbuffer cbuffer_x_8 : register(b1) {
  uint4 x_8[2];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float4 color = (0.0f).xxxx;
  float x_29 = asfloat(x_6[0u].x);
  float x_31 = asfloat(x_6[0u].x);
  float x_33 = asfloat(x_6[0u].x);
  float x_35 = asfloat(x_6[1u].x);
  color = float4(x_29, x_31, x_33, x_35);
  int x_38 = asint(x_8[1u].x);
  switch(((1 | x_38) ^ 1)) {
    case 0:
    {
      int x_44 = asint(x_8[0u].x);
      float x_46 = asfloat(x_6[1u].x);
      color[x_44] = x_46;
      break;
    }
    default:
    {
      break;
    }
  }
  float4 x_48 = color;
  x_GLF_color = x_48;
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
internal error: compilation aborted unexpectedly


tint executable returned error: exit status 1
