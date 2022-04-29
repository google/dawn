SKIP: FAILED - see https://github.com/microsoft/DirectXShaderCompiler/issues/4422

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool x_21_phi = false;
  x_21_phi = false;
  [loop] while (true) {
    bool x_25 = false;
    bool x_25_phi = false;
    bool x_30_phi = false;
    x_25_phi = x_21_phi;
    [loop] while (true) {
      x_25 = x_25_phi;
      x_30_phi = x_25;
      if ((1 < 0)) {
      } else {
        break;
      }
      x_30_phi = true;
      break;
      {
        x_25_phi = false;
      }
    }
    if (x_30_phi) {
      break;
    }
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    break;
    {
      x_21_phi = false;
    }
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
Internal compiler error: access violation. Attempted to read from address 0x0000000000000048

