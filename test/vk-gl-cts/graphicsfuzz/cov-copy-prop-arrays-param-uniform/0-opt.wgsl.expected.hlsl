struct Array {
  int values[2];
};

cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool x_52 = false;
  int x_17 = 0;
  int x_18 = 0;
  int x_16[2] = (int[2])0;
  Array param = (Array)0;
  int x_20 = 0;
  int x_21_phi = 0;
  const int x_12 = asint(x_8[0].x);
  int x_23_1[2] = x_16;
  x_23_1[0u] = x_12;
  x_16 = x_23_1;
  const Array tint_symbol_2 = {x_16};
  param = tint_symbol_2;
  x_52 = false;
  while (true) {
    int x_20_phi = 0;
    bool x_67_phi = false;
    while (true) {
      const int x_19 = param.values[x_12];
      if ((x_19 == 0)) {
        x_52 = true;
        x_17 = 42;
        x_20_phi = 42;
        x_67_phi = true;
        break;
      }
      x_20_phi = 0;
      x_67_phi = false;
      break;
    }
    x_20 = x_20_phi;
    const bool x_67 = x_67_phi;
    x_21_phi = x_20;
    if (x_67) {
      break;
    }
    x_52 = true;
    x_17 = 42;
    x_21_phi = 42;
    break;
  }
  const int x_21 = x_21_phi;
  x_18 = x_21;
  if ((x_21 == 42)) {
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}

int func_struct_Array_i1_2_1_(inout Array a) {
  bool x_74 = false;
  int x_13 = 0;
  int x_14 = 0;
  bool x_76_phi = false;
  int x_15_phi = 0;
  x_76_phi = false;
  while (true) {
    bool x_81 = false;
    bool x_81_phi = false;
    int x_14_phi = 0;
    bool x_91_phi = false;
    x_81_phi = x_76_phi;
    while (true) {
      x_81 = x_81_phi;
      const int x_10 = asint(x_8[0].x);
      const int x_11 = a.values[x_10];
      if ((x_11 == 0)) {
        x_74 = true;
        x_13 = 42;
        x_14_phi = 42;
        x_91_phi = true;
        break;
      }
      x_14_phi = 0;
      x_91_phi = x_81;
      break;
      {
        x_81_phi = false;
      }
    }
    x_14 = x_14_phi;
    const bool x_91 = x_91_phi;
    x_15_phi = x_14;
    if (x_91) {
      break;
    }
    x_74 = true;
    x_13 = 42;
    x_15_phi = 42;
    break;
    {
      x_76_phi = false;
    }
  }
  return x_15_phi;
}
