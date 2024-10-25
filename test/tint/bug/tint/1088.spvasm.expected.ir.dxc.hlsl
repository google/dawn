struct main_out {
  float4 gl_Position;
  float2 vUV_1;
};

struct main_outputs {
  float2 main_out_vUV_1 : TEXCOORD0;
  float4 main_out_gl_Position : SV_Position;
};

struct main_inputs {
  float3 position_1_param : TEXCOORD0;
  float3 normal_param : TEXCOORD1;
  float2 uv_param : TEXCOORD2;
};


static float3 position_1 = (0.0f).xxx;
cbuffer cbuffer_x_14 : register(b2, space2) {
  uint4 x_14[17];
};
static float2 vUV = (0.0f).xx;
static float2 uv = (0.0f).xx;
static float3 normal = (0.0f).xxx;
static float4 gl_Position = (0.0f).xxxx;
float4x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(x_14[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(x_14[((16u + start_byte_offset) / 16u)]);
  float4 v_3 = asfloat(x_14[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_1, v_2, v_3, asfloat(x_14[((48u + start_byte_offset) / 16u)]));
}

void main_1() {
  float4 q = (0.0f).xxxx;
  float3 p = (0.0f).xxx;
  q = float4(position_1.x, position_1.y, position_1.z, 1.0f);
  p = q.xyz;
  float v_4 = p.x;
  float v_5 = asfloat(x_14[13u].x);
  float v_6 = (v_5 * position_1.y);
  p[0u] = (v_4 + sin((v_6 + asfloat(x_14[4u].x))));
  float v_7 = p.y;
  p[1u] = (v_7 + sin((asfloat(x_14[4u].x) + 4.0f)));
  float4x4 v_8 = v(0u);
  gl_Position = mul(float4(p.x, p.y, p.z, 1.0f), v_8);
  vUV = uv;
  gl_Position[1u] = (gl_Position.y * -1.0f);
}

main_out main_inner(float3 position_1_param, float2 uv_param, float3 normal_param) {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  main_out v_9 = {gl_Position, vUV};
  return v_9;
}

main_outputs main(main_inputs inputs) {
  main_out v_10 = main_inner(inputs.position_1_param, inputs.uv_param, inputs.normal_param);
  main_outputs v_11 = {v_10.vUV_1, v_10.gl_Position};
  return v_11;
}

