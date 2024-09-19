struct main_outputs {
  float4 tint_symbol_1 : SV_Target0;
};

struct main_inputs {
  float2 vUV : TEXCOORD0;
};


SamplerState tint_symbol : register(s0);
Texture2D<float4> randomTexture : register(t1);
Texture2D<float4> depthTexture : register(t2);
float4 main_inner(float2 vUV) {
  float3 random = randomTexture.Sample(tint_symbol, vUV).xyz;
  int i = int(0);
  {
    while(true) {
      if ((i < int(1))) {
      } else {
        break;
      }
      float3 offset = float3((random[0u]).xxx);
      bool v = false;
      if ((offset[0u] < 0.0f)) {
        v = true;
      } else {
        v = (offset[1u] < 0.0f);
      }
      bool v_1 = false;
      if (v) {
        v_1 = true;
      } else {
        v_1 = (offset[0u] > 1.0f);
      }
      bool v_2 = false;
      if (v_1) {
        v_2 = true;
      } else {
        v_2 = (offset[1u] > 1.0f);
      }
      if (v_2) {
        i = (i + int(1));
        {
        }
        continue;
      }
      float sampleDepth = 0.0f;
      i = (i + int(1));
      {
      }
      continue;
    }
  }
  return (1.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  main_outputs v_3 = {main_inner(inputs.vUV)};
  return v_3;
}

