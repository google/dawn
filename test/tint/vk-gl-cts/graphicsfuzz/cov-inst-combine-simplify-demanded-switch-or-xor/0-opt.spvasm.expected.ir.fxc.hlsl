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
  float v = asfloat(x_6[0u].x);
  float v_1 = asfloat(x_6[0u].x);
  float v_2 = asfloat(x_6[0u].x);
  color = float4(v, v_1, v_2, asfloat(x_6[1u].x));
  int x_38 = asint(x_8[1u].x);
  switch(((1 | x_38) ^ 1)) {
    case 0:
    {
      int x_44 = asint(x_8[0u].x);
      color[x_44] = asfloat(x_6[1u].x);
      break;
    }
    default:
    {
      break;
    }
  }
  x_GLF_color = color;
}

main_out main_inner() {
  main_1();
  main_out v_3 = {x_GLF_color};
  return v_3;
}

main_outputs main() {
  main_out v_4 = main_inner();
  main_outputs v_5 = {v_4.x_GLF_color_1};
  return v_5;
}

FXC validation failure:
internal error: compilation aborted unexpectedly


tint executable returned error: exit status 1
