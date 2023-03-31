SKIP: FAILED

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b1) {
  uint4 x_5[2];
};
cbuffer cbuffer_x_7 : register(b2) {
  uint4 x_7[1];
};
cbuffer cbuffer_x_10 : register(b0) {
  uint4 x_10[2];
};

void main_1() {
  int i = 0;
  const float x_38 = asfloat(x_5[0].x);
  x_GLF_color = float4(x_38, x_38, x_38, x_38);
  const float x_41 = asfloat(x_7[0].x);
  const float x_43 = asfloat(x_5[0].x);
  if ((x_41 > x_43)) {
    while (true) {
      const float x_53 = asfloat(x_5[1].x);
      x_GLF_color = float4(x_53, x_53, x_53, x_53);
      {
        if (false) { break; }
      }
    }
  } else {
    while (true) {
      while (true) {
        if (true) {
        } else {
          break;
        }
        const int x_13 = asint(x_10[1].x);
        i = x_13;
        while (true) {
          const int x_14 = i;
          const int x_15 = asint(x_10[0].x);
          if ((x_14 < x_15)) {
          } else {
            break;
          }
          const float x_73 = asfloat(x_5[1].x);
          const float x_75 = asfloat(x_5[0].x);
          const float x_77 = asfloat(x_5[0].x);
          const float x_79 = asfloat(x_5[1].x);
          x_GLF_color = float4(x_73, x_75, x_77, x_79);
          {
            const int x_16 = i;
            i = (x_16 + 1);
          }
        }
        break;
      }
      {
        const float x_82 = asfloat(x_7[0].x);
        const float x_84 = asfloat(x_5[0].x);
        if (!((x_82 > x_84))) { break; }
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
DXC validation failure:
warning: DXIL signing library (dxil.dll,libdxil.so) not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
shader.hlsl:77: error: Loop must have break.
Validation failed.



