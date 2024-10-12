struct FragmentOutput {
  float4 color;
};

struct FragmentInput {
  float2 vUv;
};

struct main_outputs {
  float4 FragmentOutput_color : SV_Target0;
};

struct main_inputs {
  float2 FragmentInput_vUv : TEXCOORD2;
};


Texture2D depthMap : register(t5, space1);
SamplerState texSampler : register(s3, space1);
FragmentOutput main_inner(FragmentInput fIn) {
  float tint_symbol = depthMap.Sample(texSampler, fIn.vUv).x;
  float3 color = float3(tint_symbol, tint_symbol, tint_symbol);
  FragmentOutput fOut = (FragmentOutput)0;
  fOut.color = float4(color, 1.0f);
  FragmentOutput v = fOut;
  return v;
}

main_outputs main(main_inputs inputs) {
  FragmentInput v_1 = {inputs.FragmentInput_vUv};
  FragmentOutput v_2 = main_inner(v_1);
  main_outputs v_3 = {v_2.color};
  return v_3;
}

