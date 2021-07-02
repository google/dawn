cbuffer cbuffer_uniforms : register(b0, space0) {
  uint4 uniforms[1];
};

struct VertexOutputs {
  float2 texcoords;
  float4 position;
};
struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float2 texcoords : TEXCOORD0;
  float4 position : SV_Position;
};
struct tint_array_wrapper {
  float2 arr[3];
};

tint_symbol_2 vs_main(tint_symbol_1 tint_symbol) {
  const uint VertexIndex = tint_symbol.VertexIndex;
  tint_array_wrapper texcoord = {{float2(-0.5f, 0.0f), float2(1.5f, 0.0f), float2(0.5f, 2.0f)}};
  VertexOutputs output = (VertexOutputs)0;
  output.position = float4(((texcoord.arr[VertexIndex] * 2.0f) - float2(1.0f, 1.0f)), 0.0f, 1.0f);
  const uint scalar_offset = (4u) / 4;
  bool flipY = (asfloat(uniforms[scalar_offset / 4][scalar_offset % 4]) < 0.0f);
  if (flipY) {
    const uint scalar_offset_1 = (0u) / 4;
    uint4 ubo_load = uniforms[scalar_offset_1 / 4];
    const uint scalar_offset_2 = (8u) / 4;
    uint4 ubo_load_1 = uniforms[scalar_offset_2 / 4];
    output.texcoords = ((((texcoord.arr[VertexIndex] * asfloat(((scalar_offset_1 & 2) ? ubo_load.zw : ubo_load.xy))) + asfloat(((scalar_offset_2 & 2) ? ubo_load_1.zw : ubo_load_1.xy))) * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f));
  } else {
    const uint scalar_offset_3 = (0u) / 4;
    uint4 ubo_load_2 = uniforms[scalar_offset_3 / 4];
    const uint scalar_offset_4 = (8u) / 4;
    uint4 ubo_load_3 = uniforms[scalar_offset_4 / 4];
    output.texcoords = ((((texcoord.arr[VertexIndex] * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f)) * asfloat(((scalar_offset_3 & 2) ? ubo_load_2.zw : ubo_load_2.xy))) + asfloat(((scalar_offset_4 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
  }
  const tint_symbol_2 tint_symbol_8 = {output.texcoords, output.position};
  return tint_symbol_8;
}

SamplerState mySampler : register(s1, space0);
Texture2D<float4> myTexture : register(t2, space0);

struct tint_symbol_4 {
  float2 texcoord : TEXCOORD0;
};
struct tint_symbol_5 {
  float4 value : SV_Target0;
};

tint_symbol_5 fs_main(tint_symbol_4 tint_symbol_3) {
  const float2 texcoord = tint_symbol_3.texcoord;
  float2 clampedTexcoord = clamp(texcoord, float2(0.0f, 0.0f), float2(1.0f, 1.0f));
  if (!(all((clampedTexcoord == texcoord)))) {
    discard;
  }
  float4 srcColor = myTexture.Sample(mySampler, texcoord);
  const tint_symbol_5 tint_symbol_9 = {srcColor};
  return tint_symbol_9;
}
