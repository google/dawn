SKIP: FAILED

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};

void main_1() {
  int m = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const float x_30 = asfloat(x_5[0].x);
  const float x_32 = asfloat(x_5[0].y);
  if ((x_30 > x_32)) {
    while (true) {
      {
        if (false) {
        } else {
          break;
        }
      }
    }
    m = 1;
    while (true) {
      if (true) {
      } else {
        break;
      }
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
error: validation errors
T:\tmp\u7ck.0:39: error: Loop must have break.
Validation failed.



