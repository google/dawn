cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 I = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 N = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 R = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const int x_40 = asint(x_6[2].x);
  const int x_43 = asint(x_6[3].x);
  const int x_46 = asint(x_6[4].x);
  I = asfloat(uint4(asuint(x_40), asuint(x_43), asuint(x_46), 92985u));
  const float x_51 = asfloat(x_9[1].x);
  N = float4(x_51, x_51, x_51, x_51);
  R = reflect(I, float4(0.5f, 0.5f, 0.5f, 0.5f));
  const float4 x_55 = I;
  const float x_57 = asfloat(x_9[2].x);
  ref = (x_55 - (N * (x_57 * dot(N, I))));
  const float4 x_65 = R;
  const float4 x_66 = ref;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_69 = asfloat(x_9[scalar_offset / 4][scalar_offset % 4]);
  if ((distance(x_65, x_66) < x_69)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_75 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_78 = asint(x_6[1].x);
    const int x_81 = asint(x_6[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_84 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_75), float(x_78), float(x_81), float(x_84));
  } else {
    const int x_88 = asint(x_6[1].x);
    const float x_89 = float(x_88);
    x_GLF_color = float4(x_89, x_89, x_89, x_89);
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
