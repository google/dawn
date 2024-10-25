struct VertexOutputs {
  float2 texcoords;
  float4 position;
};

struct vs_main_outputs {
  float2 VertexOutputs_texcoords : TEXCOORD0;
  float4 VertexOutputs_position : SV_Position;
};

struct vs_main_inputs {
  uint VertexIndex : SV_VertexID;
};

struct fs_main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct fs_main_inputs {
  float2 texcoord : TEXCOORD0;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[1];
};
SamplerState mySampler : register(s1);
Texture2D<float4> myTexture : register(t2);
static bool continue_execution = true;
VertexOutputs vs_main_inner(uint VertexIndex) {
  float2 texcoord[3] = {float2(-0.5f, 0.0f), float2(1.5f, 0.0f), float2(0.5f, 2.0f)};
  VertexOutputs output = (VertexOutputs)0;
  output.position = float4(((texcoord[VertexIndex] * 2.0f) - (1.0f).xx), 0.0f, 1.0f);
  bool flipY = (asfloat(uniforms[0u].y) < 0.0f);
  if (flipY) {
    float2 v = texcoord[VertexIndex];
    float2 v_1 = (v * asfloat(uniforms[0u].xy));
    output.texcoords = (((v_1 + asfloat(uniforms[0u].zw)) * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f));
  } else {
    float2 v_2 = ((texcoord[VertexIndex] * float2(1.0f, -1.0f)) + float2(0.0f, 1.0f));
    float2 v_3 = (v_2 * asfloat(uniforms[0u].xy));
    output.texcoords = (v_3 + asfloat(uniforms[0u].zw));
  }
  VertexOutputs v_4 = output;
  return v_4;
}

float4 fs_main_inner(float2 texcoord) {
  float2 clampedTexcoord = clamp(texcoord, (0.0f).xx, (1.0f).xx);
  if (!(all((clampedTexcoord == texcoord)))) {
    continue_execution = false;
  }
  float4 srcColor = (0.0f).xxxx;
  return srcColor;
}

vs_main_outputs vs_main(vs_main_inputs inputs) {
  VertexOutputs v_5 = vs_main_inner(inputs.VertexIndex);
  vs_main_outputs v_6 = {v_5.texcoords, v_5.position};
  return v_6;
}

fs_main_outputs fs_main(fs_main_inputs inputs) {
  fs_main_outputs v_7 = {fs_main_inner(inputs.texcoord)};
  if (!(continue_execution)) {
    discard;
  }
  return v_7;
}

