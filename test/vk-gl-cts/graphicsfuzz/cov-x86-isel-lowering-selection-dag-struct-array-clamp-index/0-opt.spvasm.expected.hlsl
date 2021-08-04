struct S {
  int a;
  int b;
  int c;
};

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  S A[2] = (S[2])0;
  const int x_29 = asint(x_7[1].x);
  const int x_31 = asint(x_7[1].x);
  const int x_33 = asint(x_7[1].x);
  const int x_35 = asint(x_7[1].x);
  const S tint_symbol_2 = {x_31, x_33, x_35};
  A[x_29] = tint_symbol_2;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_39 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
  const int x_41 = asint(x_7[1].x);
  const int x_43 = asint(x_7[1].x);
  const int x_45 = asint(x_7[1].x);
  const S tint_symbol_3 = {x_41, x_43, x_45};
  A[x_39] = tint_symbol_3;
  const int x_49 = asint(x_7[1].x);
  const int x_51 = A[x_49].b;
  const int x_53 = asint(x_7[1].x);
  if ((x_51 == x_53)) {
    const int x_58 = asint(x_7[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_61 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    A[clamp(x_58, 1, 2)].b = x_61;
  }
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_64 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const int x_66 = A[x_64].b;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_68 = asint(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  if ((x_66 == x_68)) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_74 = asint(x_7[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const int x_77 = asint(x_7[1].x);
    const int x_80 = asint(x_7[1].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_83 = asint(x_7[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(float(x_74), float(x_77), float(x_80), float(x_83));
  } else {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_87 = asint(x_7[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const float x_88 = float(x_87);
    x_GLF_color = float4(x_88, x_88, x_88, x_88);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
