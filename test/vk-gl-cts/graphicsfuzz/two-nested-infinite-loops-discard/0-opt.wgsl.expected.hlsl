SKIP: FAILED

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 mand_() {
  int k = 0;
  k = 0;
  while (true) {
    if (true) {
    } else {
      break;
    }
    discard;
  }
  return float3(1.0f, 1.0f, 1.0f);
}

void main_1() {
  int j = 0;
  const float x_37 = asfloat(x_7[0].x);
  const float x_39 = asfloat(x_7[0].y);
  if ((x_37 > x_39)) {
    j = 0;
    {
      for(; true; const float3 x_46 = mand_()) {
      }
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
C:\src\tint\test\Shader@0x000001DB32C08F00(26,19-23): error X3000: syntax error: unexpected token 'const'
C:\src\tint\test\Shader@0x000001DB32C08F00(41,17): error X3000: syntax error: unexpected token '('
C:\src\tint\test\Shader@0x000001DB32C08F00(45,3-23): error X3079: 'main_1': void functions cannot return a value

