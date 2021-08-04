cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float a = 0.0f;
  int i = 0;
  float b = 0.0f;
  float c = 0.0f;
  float d = 0.0f;
  bool x_67 = false;
  bool x_68_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_37 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  a = x_37;
  const int x_39 = asint(x_9[1].x);
  i = x_39;
  while (true) {
    const int x_44 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_46 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_44 < x_46)) {
    } else {
      break;
    }
    b = a;
    c = b;
    d = asin(c);
    const float x_54 = asfloat(x_6[1].x);
    c = x_54;
    a = d;
    {
      i = (i + 1);
    }
  }
  const float x_59 = asfloat(x_6[2].x);
  const bool x_61 = (x_59 < a);
  x_68_phi = x_61;
  if (x_61) {
    const float x_64 = a;
    const float x_66 = asfloat(x_6[3].x);
    x_67 = (x_64 < x_66);
    x_68_phi = x_67;
  }
  if (x_68_phi) {
    const int x_73 = asint(x_9[2].x);
    const int x_76 = asint(x_9[1].x);
    const int x_79 = asint(x_9[1].x);
    const int x_82 = asint(x_9[2].x);
    x_GLF_color = float4(float(x_73), float(x_76), float(x_79), float(x_82));
  } else {
    const int x_86 = asint(x_9[1].x);
    const float x_87 = float(x_86);
    x_GLF_color = float4(x_87, x_87, x_87, x_87);
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
