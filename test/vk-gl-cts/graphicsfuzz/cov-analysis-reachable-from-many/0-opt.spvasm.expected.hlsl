cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float func_() {
  float s = 0.0f;
  int i = 0;
  int j = 0;
  s = 2.0f;
  i = 0;
  while (true) {
    const int x_47 = i;
    const int x_49 = asint(x_8[0].x);
    if ((x_47 < (x_49 + 1))) {
    } else {
      break;
    }
    s = (s + 3.0f);
    j = 0;
    {
      for(; (j < 10); j = (j + 1)) {
        const int x_63 = asint(x_8[0].x);
        if ((x_63 == 1)) {
          discard;
        }
      }
    }
    {
      i = (i + 1);
    }
  }
  return s;
}

void main_1() {
  float4 c = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float x_34 = func_();
  c = float4(x_34, 0.0f, 0.0f, 1.0f);
  const float x_36 = func_();
  if ((x_36 == 5.0f)) {
    x_GLF_color = c;
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
