cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int arr[10] = (int[10])0;
  int a = 0;
  int i = 0;
  const int tint_symbol_2[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  arr = tint_symbol_2;
  a = 0;
  const int x_42 = asint(x_7[1].x);
  const int x_44 = arr[x_42];
  if ((x_44 == 2)) {
    const int x_49 = asint(x_7[2].x);
    i = x_49;
    while (true) {
      const int x_54 = i;
      const uint scalar_offset = ((16u * uint(0))) / 4;
      const int x_56 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
      if ((x_54 < x_56)) {
      } else {
        break;
      }
      {
        i = (i + 1);
      }
    }
    a = (a + 1);
  }
  const int x_63 = a;
  const int x_66 = asint(x_7[2].x);
  if (((-1 % x_63) == x_66)) {
    const int x_71 = asint(x_7[1].x);
    const int x_75 = asint(x_7[2].x);
    arr[int2(x_71, x_71).y] = x_75;
  }
  const int x_78 = asint(x_7[1].x);
  const int x_80 = arr[x_78];
  const int x_82 = asint(x_7[2].x);
  if ((x_80 == x_82)) {
    const int x_88 = asint(x_7[1].x);
    const int x_91 = asint(x_7[2].x);
    const int x_94 = asint(x_7[2].x);
    const int x_97 = asint(x_7[1].x);
    x_GLF_color = float4(float(x_88), float(x_91), float(x_94), float(x_97));
  } else {
    const int x_101 = asint(x_7[2].x);
    const float x_102 = float(x_101);
    x_GLF_color = float4(x_102, x_102, x_102, x_102);
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
