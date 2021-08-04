cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int a[2] = (int[2])0;
  const int x_32 = asint(x_6[2].x);
  i = x_32;
  while (true) {
    const int x_37 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_39 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_37 < x_39)) {
    } else {
      break;
    }
    const int x_43 = asint(x_6[1].x);
    const int x_44 = i;
    const int x_46 = asint(x_6[3].x);
    const int tint_symbol_2[2] = {x_43, (int2(x_44, x_44) % int2(3, x_46)).y};
    a = tint_symbol_2;
    {
      i = (i + 1);
    }
  }
  const int x_55 = asint(x_6[2].x);
  const int x_57 = a[x_55];
  const int x_60 = asint(x_6[1].x);
  const int x_63 = asint(x_6[1].x);
  const int x_66 = asint(x_6[2].x);
  const int x_68 = a[x_66];
  x_GLF_color = float4(float(x_57), float(x_60), float(x_63), float(x_68));
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
