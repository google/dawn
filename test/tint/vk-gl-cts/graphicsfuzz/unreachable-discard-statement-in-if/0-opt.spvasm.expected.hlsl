SKIP: FAILED

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 computePoint_() {
  if (true) {
    const float x_48 = asfloat(x_7[0].x);
    const float x_50 = asfloat(x_7[0].y);
    if ((x_48 > x_50)) {
      discard;
    }
    return float3(0.0f, 0.0f, 0.0f);
  }
  float3 unused;
  return unused;
}

void main_1() {
  bool x_34 = false;
  [loop] while (true) {
    const float3 x_36 = computePoint_();
    const float x_41 = gl_FragCoord.x;
    if ((x_41 < 0.0f)) {
      x_34 = true;
      break;
    }
    const float3 x_45 = computePoint_();
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    x_34 = true;
    break;
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
C:\src\tint\test\Shader@0x0000014F8C58FE70(22,10-21): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
C:\src\tint\test\Shader@0x0000014F8C58FE70(11,10-13): internal error: invalid access of unbound variable

