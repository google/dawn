SKIP: FAILED

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  const int x_27 = asint(x_6[3].x);
  a = x_27;
  i = 0;
  {
    for(; (i < 3); i = (i + 1)) {
      const int x_35 = i;
      const int x_37 = asint(x_6[1].x);
      if ((x_35 == x_37)) {
        a = (a + 1);
      } else {
        a = (a / i);
      }
    }
  }
  const int x_49 = a;
  const int x_51 = asint(x_6[2].x);
  if ((x_49 == x_51)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    const int x_60 = asint(x_6[1].x);
    const int x_63 = asint(x_6[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_66 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(float(x_57), float(x_60), float(x_63), float(x_66));
  } else {
    const int x_70 = asint(x_6[1].x);
    const float x_71 = float(x_70);
    x_GLF_color = float4(x_71, x_71, x_71, x_71);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
O:\src\chrome\src\third_party\dawn\third_party\tint\test\Shader@0x0000029A80398B60(19,14-18): error X4010: Unsigned integer divide by zero
O:\src\chrome\src\third_party\dawn\third_party\tint\test\Shader@0x0000029A80398B60(19,14-18): warning X3556: integer divides may be much slower, try using uints if possible.

