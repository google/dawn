struct FragmentInput {
  float4 position;
  float4 view_position;
  float4 normal;
  float2 uv;
  float4 color;
};

struct Uniforms {
  float4x4 worldView;
  float4x4 proj;
  uint numPointLights;
  uint color_source;
  float4 color;
};

struct FragmentOutput {
  float4 color;
};

struct main_outputs {
  float4 FragmentOutput_color : SV_Target0;
};

struct main_inputs {
  float4 FragmentInput_view_position : TEXCOORD0;
  float4 FragmentInput_normal : TEXCOORD1;
  float2 FragmentInput_uv : TEXCOORD2;
  float4 FragmentInput_color : TEXCOORD3;
  float4 FragmentInput_position : SV_Position;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[10];
};
ByteAddressBuffer pointLights : register(t1);
SamplerState mySampler : register(s2);
Texture2D<float4> myTexture : register(t3);
float4 getColor(FragmentInput fragment) {
  float4 color = (0.0f).xxxx;
  if ((uniforms[8u].y == 0u)) {
    color = fragment.color;
  } else {
    if ((uniforms[8u].y == 1u)) {
      color = fragment.normal;
      color[3u] = 1.0f;
    } else {
      if ((uniforms[8u].y == 2u)) {
        color = asfloat(uniforms[9u]);
      } else {
        if ((uniforms[8u].y == 3u)) {
          color = myTexture.Sample(mySampler, fragment.uv);
        }
      }
    }
  }
  return color;
}

float4x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(uniforms[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(uniforms[((16u + start_byte_offset) / 16u)]);
  float4 v_3 = asfloat(uniforms[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_1, v_2, v_3, asfloat(uniforms[((48u + start_byte_offset) / 16u)]));
}

Uniforms v_4(uint start_byte_offset) {
  float4x4 v_5 = v(start_byte_offset);
  float4x4 v_6 = v((64u + start_byte_offset));
  uint v_7 = uniforms[((128u + start_byte_offset) / 16u)][(((128u + start_byte_offset) % 16u) / 4u)];
  uint v_8 = uniforms[((132u + start_byte_offset) / 16u)][(((132u + start_byte_offset) % 16u) / 4u)];
  Uniforms v_9 = {v_5, v_6, v_7, v_8, asfloat(uniforms[((144u + start_byte_offset) / 16u)])};
  return v_9;
}

FragmentOutput main_inner(FragmentInput fragment) {
  FragmentOutput output = (FragmentOutput)0;
  output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  v_4(0u);
  FragmentOutput v_10 = output;
  return v_10;
}

main_outputs main(main_inputs inputs) {
  FragmentInput v_11 = {float4(inputs.FragmentInput_position.xyz, (1.0f / inputs.FragmentInput_position[3u])), inputs.FragmentInput_view_position, inputs.FragmentInput_normal, inputs.FragmentInput_uv, inputs.FragmentInput_color};
  FragmentOutput v_12 = main_inner(v_11);
  main_outputs v_13 = {v_12.color};
  return v_13;
}

