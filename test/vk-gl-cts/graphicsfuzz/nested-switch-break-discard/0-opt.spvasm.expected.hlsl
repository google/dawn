static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  const float x_30 = asfloat(x_6[0].x);
  switch(int(x_30)) {
    case 0: {
      switch(1) {
        case 1: {
          const float x_38 = gl_FragCoord.y;
          if ((x_38 >= 0.0f)) {
            x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
          }
          discard;
          break;
        }
        default: {
          break;
        }
      }
      /* fallthrough */
      {
      }
      break;
    }
    case 42: {
      break;
    }
    default: {
      break;
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
