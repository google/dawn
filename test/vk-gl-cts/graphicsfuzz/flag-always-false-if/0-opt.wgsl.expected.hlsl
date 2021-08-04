cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int loop_count = 0;
  loop_count = 0;
  const float x_33 = asfloat(x_7[0].x);
  const float x_35 = asfloat(x_7[0].y);
  const bool x_36 = (x_33 > x_35);
  if (x_36) {
    return;
  }
  const float x_40 = gl_FragCoord.x;
  const bool x_41 = (x_40 < 0.0f);
  {
    for(; (loop_count < 100); loop_count = (loop_count + 1)) {
      if (x_36) {
        break;
      }
      if (x_36) {
        x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
      } else {
        if (x_41) {
          return;
        }
      }
      if (x_36) {
        x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
      } else {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      }
      if (x_36) {
        return;
      }
      if (x_41) {
        {
          for(; (loop_count < 100); loop_count = (loop_count + 1)) {
          }
        }
      }
    }
  }
  if ((loop_count >= 100)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
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
