struct VertexInput {
  float4 position;
  float3 normal;
  float4 tangent;
  float2 texcoord;
  uint4 joints;
  float4 weights;
  float4 instance0;
  float4 instance1;
  float4 instance2;
  float4 instance3;
  float4 instanceColor;
};
struct VertexOutput {
  float4 position;
  float3 worldPos;
  float3 view;
  float2 texcoord;
  float2 texcoord2;
  float4 color;
  float4 instanceColor;
  float3 normal;
  float3 tangent;
  float3 bitangent;
};

cbuffer cbuffer_camera : register(b0, space0) {
  uint4 camera[14];
};

float4x4 getInstanceMatrix(VertexInput input) {
  return float4x4(input.instance0, input.instance1, input.instance2, input.instance3);
}

ByteAddressBuffer joint : register(t1, space0);
ByteAddressBuffer inverseBind : register(t2, space0);

float4x4 tint_symbol_3(ByteAddressBuffer buffer, uint offset) {
  return float4x4(asfloat(buffer.Load4((offset + 0u))), asfloat(buffer.Load4((offset + 16u))), asfloat(buffer.Load4((offset + 32u))), asfloat(buffer.Load4((offset + 48u))));
}

float4x4 getSkinMatrix(VertexInput input) {
  const float4x4 joint0 = mul(tint_symbol_3(inverseBind, (64u * input.joints.x)), tint_symbol_3(joint, (64u * input.joints.x)));
  const float4x4 joint1 = mul(tint_symbol_3(inverseBind, (64u * input.joints.y)), tint_symbol_3(joint, (64u * input.joints.y)));
  const float4x4 joint2 = mul(tint_symbol_3(inverseBind, (64u * input.joints.z)), tint_symbol_3(joint, (64u * input.joints.z)));
  const float4x4 joint3 = mul(tint_symbol_3(inverseBind, (64u * input.joints.w)), tint_symbol_3(joint, (64u * input.joints.w)));
  const float4x4 skinMatrix = ((((joint0 * input.weights.x) + (joint1 * input.weights.y)) + (joint2 * input.weights.z)) + (joint3 * input.weights.w));
  return skinMatrix;
}

struct tint_symbol_1 {
  float4 position : TEXCOORD0;
  float3 normal : TEXCOORD1;
  float4 tangent : TEXCOORD2;
  float2 texcoord : TEXCOORD3;
  uint4 joints : TEXCOORD6;
  float4 weights : TEXCOORD7;
  float4 instance0 : TEXCOORD8;
  float4 instance1 : TEXCOORD9;
  float4 instance2 : TEXCOORD10;
  float4 instance3 : TEXCOORD11;
  float4 instanceColor : TEXCOORD12;
};
struct tint_symbol_2 {
  float3 worldPos : TEXCOORD0;
  float3 view : TEXCOORD1;
  float2 texcoord : TEXCOORD2;
  float2 texcoord2 : TEXCOORD3;
  float4 color : TEXCOORD4;
  float4 instanceColor : TEXCOORD5;
  float3 normal : TEXCOORD6;
  float3 tangent : TEXCOORD7;
  float3 bitangent : TEXCOORD8;
  float4 position : SV_Position;
};

float4x4 tint_symbol_6(uint4 buffer[14], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

VertexOutput vertexMain_inner(VertexInput input) {
  VertexOutput output = (VertexOutput)0;
  const float4x4 modelMatrix = getSkinMatrix(input);
  output.normal = normalize(mul(float4(input.normal, 0.0f), modelMatrix).xyz);
  output.tangent = normalize(mul(float4(input.tangent.xyz, 0.0f), modelMatrix).xyz);
  output.bitangent = (cross(output.normal, output.tangent) * input.tangent.w);
  output.color = float4((1.0f).xxxx);
  output.texcoord = input.texcoord;
  output.instanceColor = input.instanceColor;
  const float4 modelPos = mul(input.position, modelMatrix);
  output.worldPos = modelPos.xyz;
  output.view = (asfloat(camera[12].xyz) - modelPos.xyz);
  output.position = mul(modelPos, mul(tint_symbol_6(camera, 128u), tint_symbol_6(camera, 0u)));
  return output;
}

tint_symbol_2 vertexMain(tint_symbol_1 tint_symbol) {
  const VertexInput tint_symbol_8 = {tint_symbol.position, tint_symbol.normal, tint_symbol.tangent, tint_symbol.texcoord, tint_symbol.joints, tint_symbol.weights, tint_symbol.instance0, tint_symbol.instance1, tint_symbol.instance2, tint_symbol.instance3, tint_symbol.instanceColor};
  const VertexOutput inner_result = vertexMain_inner(tint_symbol_8);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.position = inner_result.position;
  wrapper_result.worldPos = inner_result.worldPos;
  wrapper_result.view = inner_result.view;
  wrapper_result.texcoord = inner_result.texcoord;
  wrapper_result.texcoord2 = inner_result.texcoord2;
  wrapper_result.color = inner_result.color;
  wrapper_result.instanceColor = inner_result.instanceColor;
  wrapper_result.normal = inner_result.normal;
  wrapper_result.tangent = inner_result.tangent;
  wrapper_result.bitangent = inner_result.bitangent;
  return wrapper_result;
}
