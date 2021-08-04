cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int A[2] = (int[2])0;
  int a = 0;
  const int x_30 = asint(x_6[1].x);
  i = x_30;
  while (true) {
    const int x_35 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_37 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_35 < x_37)) {
    } else {
      break;
    }
    A[i] = i;
    {
      i = (i + 1);
    }
  }
  const int x_46 = asint(x_6[1].x);
  const int x_48 = A[x_46];
  const int x_51 = asint(x_6[2].x);
  const int x_53 = A[x_51];
  a = min(~(x_48), ~(x_53));
  const int x_57 = asint(x_6[1].x);
  const float x_58 = float(x_57);
  x_GLF_color = float4(x_58, x_58, x_58, x_58);
  const int x_60 = a;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_62 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_60 == -(x_62))) {
    const int x_68 = asint(x_6[2].x);
    const int x_71 = asint(x_6[1].x);
    const int x_74 = asint(x_6[1].x);
    const int x_77 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_68), float(x_71), float(x_74), float(x_77));
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
