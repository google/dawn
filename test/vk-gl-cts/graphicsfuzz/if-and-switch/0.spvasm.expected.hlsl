cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float data[2] = (float[2])0;
  float x_32 = 0.0f;
  x_32 = asfloat(x_6[0].x);
  data[0] = x_32;
  data[1] = x_32;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const float x_35 = data[1];
  if ((x_35 > 1.0f)) {
    float x_43_phi = 0.0f;
    const int x_39 = int(x_32);
    x_43_phi = 0.0f;
    switch(x_39) {
      case 0: {
        x_43_phi = 1.0f;
        /* fallthrough */
        {
          data[x_39] = x_43_phi;
          x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        break;
      }
      case 1: {
        data[x_39] = x_43_phi;
        x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        break;
      }
      default: {
        break;
      }
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
