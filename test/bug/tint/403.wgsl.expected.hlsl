SKIP: FAILED



Validation Failure:
struct vertexUniformBuffer1 {
  float2x2 transform1;
};
struct vertexUniformBuffer2 {
  float2x2 transform2;
};
struct tint_symbol_1 {
  int gl_VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

ConstantBuffer<vertexUniformBuffer1> x_20 : register(b0, space0);
ConstantBuffer<vertexUniformBuffer2> x_26 : register(b0, space1);

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const int gl_VertexIndex = tint_symbol.gl_VertexIndex;
  float2 indexable[3] = {float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f)};
  const float2x2 x_23 = x_20.transform1;
  const float2x2 x_28 = x_26.transform2;
  const int x_46 = gl_VertexIndex;
  const float2 tint_symbol_3[3] = {float2(-1.0f, 1.0f), float2(1.0f, 1.0f), float2(-1.0f, -1.0f)};
  indexable = tint_symbol_3;
  const float2 x_51 = indexable[x_46];
  const float2 x_52 = mul(x_51, float2x2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])));
  const tint_symbol_2 tint_symbol_4 = {float4(x_52.x, x_52.y, 0.0f, 1.0f)};
  return tint_symbol_4;
}


warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
error: SV_VertexID must be uint.
Validation failed.


