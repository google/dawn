cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool x_68 = false;
  int x_29 = 0;
  int x_30 = 0;
  int x_31 = 0;
  int globalNumbers[10] = (int[10])0;
  int x_17 = 0;
  int acc = 0;
  int i_1 = 0;
  int localNumbers[2] = (int[2])0;
  int param = 0;
  int x_24 = 0;
  int x_24_phi = 0;
  int x_23_phi = 0;
  acc = 0;
  i_1 = 0;
  x_24_phi = 0;
  x_23_phi = 0;
  while (true) {
    int x_33 = 0;
    int x_92 = 0;
    bool x_76_phi = false;
    int x_34_phi = 0;
    int x_25_phi = 0;
    x_24 = x_24_phi;
    const int x_23 = x_23_phi;
    if ((x_23 < 4)) {
    } else {
      break;
    }
    x_68 = false;
    x_76_phi = false;
    while (true) {
      bool x_81 = false;
      int x_32 = 0;
      bool x_81_phi = false;
      int x_32_phi = 0;
      int x_33_phi = 0;
      bool x_90_phi = false;
      const bool x_76 = x_76_phi;
      x_30 = 0;
      x_81_phi = x_76;
      x_32_phi = 0;
      while (true) {
        x_81 = x_81_phi;
        x_32 = x_32_phi;
        const float x_86 = asfloat(x_8[0].x);
        x_33_phi = 0;
        x_90_phi = x_81;
        if ((x_32 < int(x_86))) {
        } else {
          break;
        }
        x_68 = true;
        x_29 = x_32;
        x_33_phi = x_32;
        x_90_phi = true;
        break;
        {
          x_81_phi = false;
          x_32_phi = 0;
        }
      }
      x_33 = x_33_phi;
      const bool x_90 = x_90_phi;
      x_34_phi = x_33;
      if (x_90) {
        break;
      }
      x_92 = 0;
      x_68 = true;
      x_29 = x_92;
      x_34_phi = x_92;
      break;
      {
        x_76_phi = false;
      }
    }
    x_31 = x_34_phi;
    const int x_93 = x_31;
    int x_22_1[2] = localNumbers;
    x_22_1[1u] = x_93;
    const int x_22[2] = x_22_1;
    localNumbers = x_22;
    globalNumbers[0] = 0;
    const int x_13 = x_22[1u];
    param = x_13;
    x_17 = 0;
    x_25_phi = 0;
    while (true) {
      const int x_25 = x_25_phi;
      if ((x_25 <= x_13)) {
      } else {
        break;
      }
      const int x_102_save = x_13;
      const int x_18 = globalNumbers[x_102_save];
      if ((x_18 <= 1)) {
        globalNumbers[x_102_save] = 1;
      }
      {
        const int x_19 = (x_25 + 1);
        x_17 = x_19;
        x_25_phi = x_19;
      }
    }
    const float x_107 = asfloat(x_8[0].x);
    const int x_14 = globalNumbers[(int(x_107) - 1)];
    const int x_15 = asint((x_24 + asint(x_14)));
    acc = x_15;
    {
      const int x_16 = (x_23 + 1);
      i_1 = x_16;
      x_24_phi = x_15;
      x_23_phi = x_16;
    }
  }
  if ((x_24 == asint(4))) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
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

int yieldsZero_() {
  bool x_116 = false;
  int x_20 = 0;
  int i = 0;
  int x_26 = 0;
  int x_134 = 0;
  bool x_118_phi = false;
  int x_27_phi = 0;
  x_118_phi = false;
  while (true) {
    bool x_123 = false;
    int x_28 = 0;
    bool x_123_phi = false;
    int x_28_phi = 0;
    int x_26_phi = 0;
    bool x_132_phi = false;
    const bool x_118 = x_118_phi;
    i = 0;
    x_123_phi = x_118;
    x_28_phi = 0;
    while (true) {
      x_123 = x_123_phi;
      x_28 = x_28_phi;
      const float x_128 = asfloat(x_8[0].x);
      x_26_phi = 0;
      x_132_phi = x_123;
      if ((x_28 < int(x_128))) {
      } else {
        break;
      }
      x_116 = true;
      x_20 = x_28;
      x_26_phi = x_28;
      x_132_phi = true;
      break;
      {
        x_123_phi = false;
        x_28_phi = 0;
      }
    }
    x_26 = x_26_phi;
    const bool x_132 = x_132_phi;
    x_27_phi = x_26;
    if (x_132) {
      break;
    }
    x_134 = 0;
    x_116 = true;
    x_20 = x_134;
    x_27_phi = x_134;
    break;
    {
      x_118_phi = false;
    }
  }
  return x_27_phi;
}
