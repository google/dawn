cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a[3] = (int[3])0;
  int b = 0;
  int c = 0;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  b = 0;
  const int x_38 = asint(x_8[0].x);
  const int x_40 = a[x_38];
  c = x_40;
  if ((c > 1)) {
    x_GLF_color = float4(0.0f, 1.0f, 1.0f, 0.0f);
    b = (b + 1);
  }
  const int x_48 = (b + 1);
  b = x_48;
  const int x_50_save = clamp(x_48, 0, 2);
  const int x_51 = a[x_50_save];
  a[x_50_save] = (x_51 + 1);
  const int x_54 = a[2];
  if ((x_54 == 4)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
