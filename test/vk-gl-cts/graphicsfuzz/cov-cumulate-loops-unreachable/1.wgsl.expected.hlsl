static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int b = 0;
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  int indexable[2] = (int[2])0;
  a = 0;
  b = 1;
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  i = 0;
  {
    for(; (i < 10); i = (i + 1)) {
      if ((i > 1)) {
        a = (a + 1);
        if (false) {
          i_1 = 0;
          while (true) {
            if ((i_1 < 10)) {
            } else {
              break;
            }
            return;
          }
        }
      }
    }
  }
  i_2 = 0;
  {
    for(; (i_2 < 10); i_2 = (i_2 + 1)) {
      const int x_65 = b;
      const int tint_symbol_2[2] = {1, 2};
      indexable = tint_symbol_2;
      const int x_67 = indexable[x_65];
      a = (a + x_67);
    }
  }
  if ((a == 28)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
