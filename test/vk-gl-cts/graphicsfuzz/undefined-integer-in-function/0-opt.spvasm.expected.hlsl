static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};

int performPartition_() {
  int GLF_live0i = 0;
  int i = 0;
  int x_11 = 0;
  int x_10_phi = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  x_10_phi = 0;
  while (true) {
    int x_11_phi = 0;
    const int x_10 = x_10_phi;
    bool x_42 = false;
    const float x_41 = asfloat(x_6[0].y);
    x_42 = (x_41 < 0.0f);
    if (x_42) {
      x_11_phi = x_10;
      {
        x_11 = x_11_phi;
        x_10_phi = x_11;
        if (false) {
        } else {
          break;
        }
      }
      continue;
    } else {
      GLF_live0i = 0;
      while (true) {
        const bool x_47 = (0 < 1);
        if (x_42) {
          break;
        }
        return 1;
      }
      if (x_42) {
        while (true) {
          return 1;
        }
        return 0;
      }
      x_11_phi = x_10;
      {
        x_11 = x_11_phi;
        x_10_phi = x_11;
        if (false) {
        } else {
          break;
        }
      }
      continue;
    }
    x_11_phi = 0;
    {
      x_11 = x_11_phi;
      x_10_phi = x_11;
      if (false) {
      } else {
        break;
      }
    }
  }
  return x_11;
}

void main_1() {
  const int x_9 = performPartition_();
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
