static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool x_29 = false;
  float x_30 = 0.0f;
  float x_31 = 0.0f;
  float x_32 = 0.0f;
  bool x_33 = false;
  float x_34 = 0.0f;
  float x_35 = 0.0f;
  float x_36 = 0.0f;
  float f = 0.0f;
  float i = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  f = 0.0f;
  i = 0.0f;
  while (true) {
    param = i;
    x_33 = false;
    while (true) {
      x_35 = param;
      while (true) {
        if ((x_35 == param)) {
          const float x_53 = x_35;
          x_33 = true;
          x_34 = x_53;
          break;
        }
        x_35 = (x_35 + 1.0f);
        {
          if ((x_35 < param)) {
          } else {
            break;
          }
        }
      }
      if (x_33) {
        break;
      }
      x_33 = true;
      x_34 = 0.0f;
      break;
    }
    const float x_61 = x_34;
    x_36 = x_61;
    f = x_61;
    param_1 = 1.0f;
    x_29 = false;
    while (true) {
      x_31 = param_1;
      while (true) {
        if ((x_31 == param_1)) {
          const float x_75 = x_31;
          x_29 = true;
          x_30 = x_75;
          break;
        }
        x_31 = (x_31 + 1.0f);
        {
          if ((x_31 < param_1)) {
          } else {
            break;
          }
        }
      }
      if (x_29) {
        break;
      }
      x_29 = true;
      x_30 = 0.0f;
      break;
    }
    const float x_83 = x_30;
    x_32 = x_83;
    const float x_85 = (i + x_83);
    i = x_85;
    if ((x_85 < 6.0f)) {
    } else {
      break;
    }
  }
  if ((f == 5.0f)) {
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}

float func_f1_(inout float x) {
  bool x_93 = false;
  float x_94 = 0.0f;
  float a = 0.0f;
  while (true) {
    const float x_96 = x;
    a = x_96;
    while (true) {
      const float x_103 = a;
      const float x_104 = x;
      if ((x_103 == x_104)) {
        const float x_108 = a;
        x_93 = true;
        x_94 = x_108;
        break;
      }
      a = (a + 1.0f);
      {
        const float x_111 = a;
        const float x_112 = x;
        if ((x_111 < x_112)) {
        } else {
          break;
        }
      }
    }
    if (x_93) {
      break;
    }
    x_93 = true;
    x_94 = 0.0f;
    break;
  }
  return x_94;
}
