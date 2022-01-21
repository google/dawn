cbuffer cbuffer_time : register(b0, space0) {
  uint4 time[1];
};
cbuffer cbuffer_uniforms : register(b1, space0) {
  uint4 uniforms[2];
};

struct VertexOutput {
  float4 Position;
  float4 v_color;
};
struct tint_symbol_1 {
  float4 position : TEXCOORD0;
  float4 color : TEXCOORD1;
};
struct tint_symbol_2 {
  float4 v_color : TEXCOORD0;
  float4 Position : SV_Position;
};

VertexOutput vert_main_inner(float4 position, float4 color) {
  float fade = ((asfloat(uniforms[1].x) + ((asfloat(time[0].x) * asfloat(uniforms[0].w)) / 10.0f)) % 1.0f);
  if ((fade < 0.5f)) {
    fade = (fade * 2.0f);
  } else {
    fade = ((1.0f - fade) * 2.0f);
  }
  float xpos = (position.x * asfloat(uniforms[0].x));
  float ypos = (position.y * asfloat(uniforms[0].x));
  float angle = ((3.141590118f * 2.0f) * fade);
  float xrot = ((xpos * cos(angle)) - (ypos * sin(angle)));
  float yrot = ((xpos * sin(angle)) + (ypos * cos(angle)));
  xpos = (xrot + asfloat(uniforms[0].y));
  ypos = (yrot + asfloat(uniforms[0].z));
  VertexOutput output = (VertexOutput)0;
  output.v_color = (float4(fade, (1.0f - fade), 0.0f, 1.0f) + color);
  output.Position = float4(xpos, ypos, 0.0f, 1.0f);
  return output;
}

tint_symbol_2 vert_main(tint_symbol_1 tint_symbol) {
  const VertexOutput inner_result = vert_main_inner(tint_symbol.position, tint_symbol.color);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.Position = inner_result.Position;
  wrapper_result.v_color = inner_result.v_color;
  return wrapper_result;
}

struct tint_symbol_4 {
  float4 v_color : TEXCOORD0;
};
struct tint_symbol_5 {
  float4 value : SV_Target0;
};

float4 frag_main_inner(float4 v_color) {
  return v_color;
}

tint_symbol_5 frag_main(tint_symbol_4 tint_symbol_3) {
  const float4 inner_result_1 = frag_main_inner(tint_symbol_3.v_color);
  tint_symbol_5 wrapper_result_1 = (tint_symbol_5)0;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
