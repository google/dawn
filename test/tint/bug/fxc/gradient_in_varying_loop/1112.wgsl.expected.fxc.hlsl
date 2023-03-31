SamplerState tint_symbol : register(s0);
Texture2D<float4> randomTexture : register(t1);
Texture2D<float4> depthTexture : register(t2);

struct tint_symbol_2 {
  float2 vUV : TEXCOORD0;
};
struct tint_symbol_3 {
  float4 value : SV_Target0;
};

float4 main_inner(float2 vUV) {
  const float3 random = randomTexture.Sample(tint_symbol, vUV).rgb;
  int i = 0;
  while (true) {
    if ((i < 1)) {
    } else {
      break;
    }
    const float3 offset = float3((random.x).xxx);
    bool tint_tmp_2 = (offset.x < 0.0f);
    if (!tint_tmp_2) {
      tint_tmp_2 = (offset.y < 0.0f);
    }
    bool tint_tmp_1 = (tint_tmp_2);
    if (!tint_tmp_1) {
      tint_tmp_1 = (offset.x > 1.0f);
    }
    bool tint_tmp = (tint_tmp_1);
    if (!tint_tmp) {
      tint_tmp = (offset.y > 1.0f);
    }
    if ((tint_tmp)) {
      i = (i + 1);
      continue;
    }
    const float sampleDepth = 0.0f;
    i = (i + 1);
  }
  return (1.0f).xxxx;
}

tint_symbol_3 main(tint_symbol_2 tint_symbol_1) {
  const float4 inner_result = main_inner(tint_symbol_1.vUV);
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
