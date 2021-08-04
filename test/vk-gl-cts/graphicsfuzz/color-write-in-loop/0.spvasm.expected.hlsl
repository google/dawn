struct S {
  int f0;
  float4x3 f1;
};

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x_51 = 0;
  int x_12_phi = 0;
  while (true) {
    S x_45 = (S)0;
    S x_45_phi = (S)0;
    int x_11_phi = 0;
    const S tint_symbol_3 = {0, float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, 0.0f))};
    x_45_phi = tint_symbol_3;
    x_11_phi = 0;
    while (true) {
      S x_46 = (S)0;
      int x_9 = 0;
      x_45 = x_45_phi;
      const int x_11 = x_11_phi;
      const float x_49 = gl_FragCoord.x;
      x_51 = ((x_49 == 0.0f) ? 1 : 2);
      if ((x_11 < x_51)) {
      } else {
        break;
      }
      {
        x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
        x_46 = x_45;
        x_46.f0 = (x_45.f0 + 1);
        x_9 = (x_11 + 1);
        x_45_phi = x_46;
        x_11_phi = x_9;
      }
    }
    if ((x_45.f0 < 1000)) {
      break;
    }
    break;
  }
  x_12_phi = 0;
  while (true) {
    int x_6 = 0;
    const int x_12 = x_12_phi;
    if ((x_12 < x_51)) {
    } else {
      break;
    }
    {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      x_6 = (x_12 + 1);
      x_12_phi = x_6;
    }
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
