struct Array {
  int values[2];
};

cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool x_50 = false;
  int x_15 = 0;
  int x_16 = 0;
  Array param = (Array)0;
  int x_19 = 0;
  int x_20_phi = 0;
  const int tint_symbol_2[2] = {0, 0};
  const Array tint_symbol_3 = {tint_symbol_2};
  param = tint_symbol_3;
  x_50 = false;
  while (true) {
    int x_19_phi = 0;
    bool x_63_phi = false;
    while (true) {
      const int x_17 = asint(x_8[0].x);
      const int x_18 = param.values[x_17];
      if ((x_18 == 1)) {
        x_50 = true;
        x_15 = 1;
        x_19_phi = 1;
        x_63_phi = true;
        break;
      }
      x_19_phi = 0;
      x_63_phi = false;
      break;
    }
    x_19 = x_19_phi;
    const bool x_63 = x_63_phi;
    x_20_phi = x_19;
    if (x_63) {
      break;
    }
    x_50 = true;
    x_15 = 1;
    x_20_phi = 1;
    break;
  }
  const int x_20 = x_20_phi;
  x_16 = x_20;
  if ((x_20 == 1)) {
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

int func_struct_Array_i1_2_1_(inout Array a) {
  bool x_70 = false;
  int x_12 = 0;
  int x_13 = 0;
  bool x_72_phi = false;
  int x_14_phi = 0;
  x_72_phi = false;
  while (true) {
    bool x_77 = false;
    bool x_77_phi = false;
    int x_13_phi = 0;
    bool x_87_phi = false;
    x_77_phi = x_72_phi;
    while (true) {
      x_77 = x_77_phi;
      const int x_10 = asint(x_8[0].x);
      const int x_11 = a.values[x_10];
      if ((x_11 == 1)) {
        x_70 = true;
        x_12 = 1;
        x_13_phi = 1;
        x_87_phi = true;
        break;
      }
      x_13_phi = 0;
      x_87_phi = x_77;
      break;
      {
        x_77_phi = false;
      }
    }
    x_13 = x_13_phi;
    const bool x_87 = x_87_phi;
    x_14_phi = x_13;
    if (x_87) {
      break;
    }
    x_70 = true;
    x_12 = 1;
    x_14_phi = 1;
    break;
    {
      x_72_phi = false;
    }
  }
  return x_14_phi;
}
