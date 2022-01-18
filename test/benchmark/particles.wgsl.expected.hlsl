static float2 rand_seed = float2(0.0f, 0.0f);

float rand() {
  rand_seed.x = frac((cos(dot(rand_seed, float2(23.140779495f, 232.616897583f))) * 136.816802979f));
  rand_seed.y = frac((cos(dot(rand_seed, float2(54.478565216f, 345.841522217f))) * 534.764526367f));
  return rand_seed.y;
}

cbuffer cbuffer_render_params : register(b0, space0) {
  uint4 render_params[6];
};

struct VertexInput {
  float3 position;
  float4 color;
  float2 quad_pos;
};
struct VertexOutput {
  float4 position;
  float4 color;
  float2 quad_pos;
};
struct tint_symbol_5 {
  float3 position : TEXCOORD0;
  float4 color : TEXCOORD1;
  float2 quad_pos : TEXCOORD2;
};
struct tint_symbol_6 {
  float4 color : TEXCOORD0;
  float2 quad_pos : TEXCOORD1;
  float4 position : SV_Position;
};

float4x4 tint_symbol_17(uint4 buffer[6], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

VertexOutput vs_main_inner(VertexInput tint_symbol) {
  float3 quad_pos = mul(tint_symbol.quad_pos, float2x3(asfloat(render_params[4].xyz), asfloat(render_params[5].xyz)));
  float3 position = (tint_symbol.position + (quad_pos * 0.01f));
  VertexOutput tint_symbol_1 = (VertexOutput)0;
  tint_symbol_1.position = mul(float4(position, 1.0f), tint_symbol_17(render_params, 0u));
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  return tint_symbol_1;
}

tint_symbol_6 vs_main(tint_symbol_5 tint_symbol_4) {
  const VertexInput tint_symbol_32 = {tint_symbol_4.position, tint_symbol_4.color, tint_symbol_4.quad_pos};
  const VertexOutput inner_result = vs_main_inner(tint_symbol_32);
  tint_symbol_6 wrapper_result = (tint_symbol_6)0;
  wrapper_result.position = inner_result.position;
  wrapper_result.color = inner_result.color;
  wrapper_result.quad_pos = inner_result.quad_pos;
  return wrapper_result;
}

struct tint_symbol_8 {
  float4 color : TEXCOORD0;
  float2 quad_pos : TEXCOORD1;
  float4 position : SV_Position;
};
struct tint_symbol_9 {
  float4 value : SV_Target0;
};

float4 fs_main_inner(VertexOutput tint_symbol) {
  float4 color = tint_symbol.color;
  color.a = (color.a * max((1.0f - length(tint_symbol.quad_pos)), 0.0f));
  return color;
}

tint_symbol_9 fs_main(tint_symbol_8 tint_symbol_7) {
  const VertexOutput tint_symbol_33 = {tint_symbol_7.position, tint_symbol_7.color, tint_symbol_7.quad_pos};
  const float4 inner_result_1 = fs_main_inner(tint_symbol_33);
  tint_symbol_9 wrapper_result_1 = (tint_symbol_9)0;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}

struct Particle {
  float3 position;
  float lifetime;
  float4 color;
  float3 velocity;
};

cbuffer cbuffer_sim_params : register(b0, space0) {
  uint4 sim_params[2];
};
RWByteAddressBuffer data : register(u1, space0);
Texture2D<float4> tint_symbol_2 : register(t2, space0);

struct tint_symbol_11 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

Particle tint_symbol_20(RWByteAddressBuffer buffer, uint offset) {
  const Particle tint_symbol_34 = {asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load((offset + 12u))), asfloat(buffer.Load4((offset + 16u))), asfloat(buffer.Load3((offset + 32u)))};
  return tint_symbol_34;
}

void tint_symbol_25(RWByteAddressBuffer buffer, uint offset, Particle value) {
  buffer.Store3((offset + 0u), asuint(value.position));
  buffer.Store((offset + 12u), asuint(value.lifetime));
  buffer.Store4((offset + 16u), asuint(value.color));
  buffer.Store3((offset + 32u), asuint(value.velocity));
}

void simulate_inner(uint3 GlobalInvocationID) {
  rand_seed = ((asfloat(sim_params[1]).xy + float2(GlobalInvocationID.xy)) * asfloat(sim_params[1]).zw);
  const uint idx = GlobalInvocationID.x;
  Particle particle = tint_symbol_20(data, (48u * idx));
  particle.velocity.z = (particle.velocity.z - (asfloat(sim_params[0].x) * 0.5f));
  particle.position = (particle.position + (asfloat(sim_params[0].x) * particle.velocity));
  particle.lifetime = (particle.lifetime - asfloat(sim_params[0].x));
  particle.color.a = smoothstep(0.0f, 0.5f, particle.lifetime);
  if ((particle.lifetime < 0.0f)) {
    int2 coord = int2(0, 0);
    {
      int3 tint_tmp;
      tint_symbol_2.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
      int level = (tint_tmp.z - 1);
      [loop] for(; (level > 0); level = (level - 1)) {
        const float4 probabilites = tint_symbol_2.Load(int3(coord, level));
        const float4 value = float4((rand()).xxxx);
        const bool4 mask = ((value >= float4(0.0f, probabilites.xyz)) & (value < probabilites));
        coord = (coord * 2);
        coord.x = (coord.x + (any(mask.yw) ? 1 : 0));
        coord.y = (coord.y + (any(mask.zw) ? 1 : 0));
      }
    }
    int2 tint_tmp_1;
    tint_symbol_2.GetDimensions(tint_tmp_1.x, tint_tmp_1.y);
    const float2 uv = (float2(coord) / float2(tint_tmp_1));
    particle.position = float3((((uv - 0.5f) * 3.0f) * float2(1.0f, -1.0f)), 0.0f);
    particle.color = tint_symbol_2.Load(int3(coord, 0));
    particle.velocity.x = ((rand() - 0.5f) * 0.100000001f);
    particle.velocity.y = ((rand() - 0.5f) * 0.100000001f);
    particle.velocity.z = (rand() * 0.300000012f);
    particle.lifetime = (0.5f + (rand() * 2.0f));
  }
  tint_symbol_25(data, (48u * idx), particle);
}

[numthreads(64, 1, 1)]
void simulate(tint_symbol_11 tint_symbol_10) {
  simulate_inner(tint_symbol_10.GlobalInvocationID);
  return;
}

cbuffer cbuffer_ubo : register(b3, space0) {
  uint4 ubo[1];
};
ByteAddressBuffer buf_in : register(t4, space0);
RWByteAddressBuffer buf_out : register(u5, space0);
Texture2D<float4> tex_in : register(t6, space0);
RWTexture2D<float4> tex_out : register(u7, space0);

struct tint_symbol_13 {
  uint3 coord : SV_DispatchThreadID;
};

void import_level_inner(uint3 coord) {
  const uint offset = (coord.x + (coord.y * ubo[0].x));
  buf_out.Store((4u * offset), asuint(tex_in.Load(int3(int2(coord.xy), 0)).w));
}

[numthreads(64, 1, 1)]
void import_level(tint_symbol_13 tint_symbol_12) {
  import_level_inner(tint_symbol_12.coord);
  return;
}

struct tint_symbol_15 {
  uint3 coord : SV_DispatchThreadID;
};

void export_level_inner(uint3 coord) {
  int2 tint_tmp_2;
  tex_out.GetDimensions(tint_tmp_2.x, tint_tmp_2.y);
  if (all((coord.xy < uint2(tint_tmp_2)))) {
    const uint dst_offset = (coord.x + (coord.y * ubo[0].x));
    const uint src_offset = ((coord.x * 2u) + ((coord.y * 2u) * ubo[0].x));
    const float a_1 = asfloat(buf_in.Load((4u * (src_offset + 0u))));
    const float b = asfloat(buf_in.Load((4u * (src_offset + 1u))));
    const float c = asfloat(buf_in.Load((4u * ((src_offset + 0u) + ubo[0].x))));
    const float d = asfloat(buf_in.Load((4u * ((src_offset + 1u) + ubo[0].x))));
    const float sum = dot(float4(a_1, b, c, d), float4((1.0f).xxxx));
    buf_out.Store((4u * dst_offset), asuint((sum / 4.0f)));
    const float4 probabilities = (float4(a_1, (a_1 + b), ((a_1 + b) + c), sum) / max(sum, 0.0001f));
    tex_out[int2(coord.xy)] = probabilities;
  }
}

[numthreads(64, 1, 1)]
void export_level(tint_symbol_15 tint_symbol_14) {
  export_level_inner(tint_symbol_14.coord);
  return;
}
