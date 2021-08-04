struct struct_base {
  int data;
  int leftIndex;
  int rightIndex;
};

static struct_base struct_array[3] = (struct_base[3])0;
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int index = 0;
  const struct_base tint_symbol_2 = {1, 1, 1};
  const struct_base tint_symbol_3 = {1, 1, 1};
  const struct_base tint_symbol_4 = {1, 1, 1};
  const struct_base tint_symbol_5[3] = {tint_symbol_2, tint_symbol_3, tint_symbol_4};
  struct_array = tint_symbol_5;
  index = 1;
  struct_array[1].rightIndex = 1;
  const int x_39 = struct_array[1].leftIndex;
  if ((x_39 == 1)) {
    const float x_45 = asfloat(x_8[0].x);
    const int x_48 = struct_array[int(x_45)].rightIndex;
    index = x_48;
  } else {
    const float x_50 = asfloat(x_8[0].y);
    const int x_53 = struct_array[int(x_50)].leftIndex;
    index = x_53;
  }
  const int x_55 = struct_array[1].leftIndex;
  if ((x_55 == 1)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
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
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
