static float3 position = float3(0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_14 : register(b2, space2) {
  uint4 x_14[17];
};
static float2 vUV = float2(0.0f, 0.0f);
static float2 uv = float2(0.0f, 0.0f);
static float3 normal = float3(0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4x4 tint_symbol_4(uint4 buffer[17], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

void main_1() {
  float4 q = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float3 p = float3(0.0f, 0.0f, 0.0f);
  const float3 x_13 = position;
  q = float4(x_13.x, x_13.y, x_13.z, 1.0f);
  const float4 x_21 = q;
  p = float3(x_21.x, x_21.y, x_21.z);
  const float x_27 = p.x;
  const uint scalar_offset_4 = ((208u + (16u * uint(0)))) / 4;
  const float x_41 = asfloat(x_14[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const float x_45 = position.y;
  const float x_49 = asfloat(x_14[4].x);
  p.x = (x_27 + sin(((x_41 * x_45) + x_49)));
  const float x_55 = p.y;
  const float x_57 = asfloat(x_14[4].x);
  p.y = (x_55 + sin((x_57 + 4.0f)));
  const float4x4 x_69 = tint_symbol_4(x_14, 0u);
  const float3 x_70 = p;
  gl_Position = mul(float4(x_70.x, x_70.y, x_70.z, 1.0f), x_69);
  vUV = uv;
  const float x_87 = gl_Position.y;
  gl_Position.y = (x_87 * -1.0f);
  return;
}

struct main_out {
  float4 gl_Position;
  float2 vUV_1;
};
struct tint_symbol_1 {
  float3 position_param : TEXCOORD0;
  float3 normal_param : TEXCOORD1;
  float2 uv_param : TEXCOORD2;
};
struct tint_symbol_2 {
  float2 vUV_1 : TEXCOORD0;
  float4 gl_Position : SV_Position;
};

main_out main_inner(float3 position_param, float2 uv_param, float3 normal_param) {
  position = position_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  const main_out tint_symbol_6 = {gl_Position, vUV};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.position_param, tint_symbol.uv_param, tint_symbol.normal_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  wrapper_result.vUV_1 = inner_result.vUV_1;
  return wrapper_result;
}
