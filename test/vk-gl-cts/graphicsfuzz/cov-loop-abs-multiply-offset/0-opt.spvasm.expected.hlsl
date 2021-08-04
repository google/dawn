cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int i = 0;
  bool x_66 = false;
  bool x_67_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_34 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  f = x_34;
  const int x_36 = asint(x_9[1].x);
  i = x_36;
  while (true) {
    const int x_41 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_43 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_41 < x_43)) {
    } else {
      break;
    }
    const float x_47 = asfloat(x_6[3].x);
    const float x_49 = f;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_53 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    f = (abs((-(x_47) * x_49)) + x_53);
    {
      i = (i + 1);
    }
  }
  const float x_57 = f;
  const float x_59 = asfloat(x_6[1].x);
  const bool x_60 = (x_57 > x_59);
  x_67_phi = x_60;
  if (x_60) {
    const float x_63 = f;
    const float x_65 = asfloat(x_6[2].x);
    x_66 = (x_63 < x_65);
    x_67_phi = x_66;
  }
  if (x_67_phi) {
    const int x_72 = asint(x_9[2].x);
    const int x_75 = asint(x_9[1].x);
    const int x_78 = asint(x_9[1].x);
    const int x_81 = asint(x_9[2].x);
    x_GLF_color = float4(float(x_72), float(x_75), float(x_78), float(x_81));
  } else {
    const int x_85 = asint(x_9[1].x);
    const float x_86 = float(x_85);
    x_GLF_color = float4(x_86, x_86, x_86, x_86);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
