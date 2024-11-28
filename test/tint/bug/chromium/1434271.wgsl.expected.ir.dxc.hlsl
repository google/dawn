struct VertexOutput {
  float4 position;
  float4 color;
  float2 quad_pos;
};

struct VertexInput {
  float3 position;
  float4 color;
  float2 quad_pos;
};

struct Particle {
  float3 position;
  float lifetime;
  float4 color;
  float2 velocity;
};

struct vertex_main_outputs {
  float4 tint_symbol_4 : SV_Position;
};

struct vs_main_outputs {
  float4 VertexOutput_color : TEXCOORD0;
  float2 VertexOutput_quad_pos : TEXCOORD1;
  float4 VertexOutput_position : SV_Position;
};

struct vs_main_inputs {
  float3 VertexInput_position : TEXCOORD0;
  float4 VertexInput_color : TEXCOORD1;
  float2 VertexInput_quad_pos : TEXCOORD2;
};

struct simulate_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

struct export_level_inputs {
  uint3 coord : SV_DispatchThreadID;
};


static float2 rand_seed = (0.0f).xx;
cbuffer cbuffer_render_params : register(b5) {
  uint4 render_params[6];
};
cbuffer cbuffer_sim_params : register(b0) {
  uint4 sim_params[2];
};
RWByteAddressBuffer data : register(u1);
Texture1D<float4> tint_symbol_2 : register(t2);
cbuffer cbuffer_ubo : register(b3) {
  uint4 ubo[1];
};
ByteAddressBuffer buf_in : register(t4);
RWByteAddressBuffer buf_out : register(u5);
Texture2D<float4> tex_in : register(t6);
RWTexture2D<float4> tex_out : register(u7);
void asinh_468a48() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t v = arg_0;
  float16_t res = log((v + sqrt(((v * v) + float16_t(1.0h)))));
}

float4 vertex_main_inner() {
  asinh_468a48();
  return (0.0f).xxxx;
}

void fragment_main() {
  asinh_468a48();
}

[numthreads(1, 1, 1)]
void rgba32uintin() {
  asinh_468a48();
}

float4x4 v_1(uint start_byte_offset) {
  return float4x4(asfloat(render_params[(start_byte_offset / 16u)]), asfloat(render_params[((16u + start_byte_offset) / 16u)]), asfloat(render_params[((32u + start_byte_offset) / 16u)]), asfloat(render_params[((48u + start_byte_offset) / 16u)]));
}

VertexOutput vs_main_inner(VertexInput tint_symbol) {
  float3 quad_pos = mul(tint_symbol.quad_pos, float2x3(asfloat(render_params[4u].xyz), asfloat(render_params[5u].xyz)));
  float3 position = (tint_symbol.position - (quad_pos + 0.00999999977648258209f));
  VertexOutput tint_symbol_1 = (VertexOutput)0;
  float4x4 v_2 = v_1(0u);
  tint_symbol_1.position = mul(float4(position, 1.0f), v_2);
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  VertexOutput v_3 = tint_symbol_1;
  return v_3;
}

void v_4(uint offset, Particle obj) {
  data.Store3((offset + 0u), asuint(obj.position));
  data.Store((offset + 12u), asuint(obj.lifetime));
  data.Store4((offset + 16u), asuint(obj.color));
  data.Store2((offset + 32u), asuint(obj.velocity));
}

Particle v_5(uint offset) {
  Particle v_6 = {asfloat(data.Load3((offset + 0u))), asfloat(data.Load((offset + 12u))), asfloat(data.Load4((offset + 16u))), asfloat(data.Load2((offset + 32u)))};
  return v_6;
}

void simulate_inner(uint3 GlobalInvocationID) {
  float2 v_7 = asfloat(sim_params[1u]).xy;
  float2 v_8 = (v_7 * float2(GlobalInvocationID.xy));
  rand_seed = (v_8 * asfloat(sim_params[1u]).zw);
  uint idx = GlobalInvocationID.x;
  uint v_9 = 0u;
  data.GetDimensions(v_9);
  Particle particle = v_5((0u + (min(idx, ((v_9 / 48u) - 1u)) * 48u)));
  uint v_10 = 0u;
  data.GetDimensions(v_10);
  Particle v_11 = particle;
  v_4((0u + (min(idx, ((v_10 / 48u) - 1u)) * 48u)), v_11);
}

void export_level_inner(uint3 coord) {
  uint2 v_12 = (0u).xx;
  tex_out.GetDimensions(v_12.x, v_12.y);
  if (all((coord.xy < uint2(v_12)))) {
    uint dst_offset = (coord.x << ((coord.y * ubo[0u].x) & 31u));
    uint src_offset = ((coord.x - 2u) + ((coord.y >> (2u & 31u)) * ubo[0u].x));
    uint v_13 = 0u;
    buf_in.GetDimensions(v_13);
    float a = asfloat(buf_in.Load((0u + (min((src_offset << (0u & 31u)), ((v_13 / 4u) - 1u)) * 4u))));
    uint v_14 = 0u;
    buf_in.GetDimensions(v_14);
    float b = asfloat(buf_in.Load((0u + (min((src_offset + 1u), ((v_14 / 4u) - 1u)) * 4u))));
    uint v_15 = 0u;
    buf_in.GetDimensions(v_15);
    float c = asfloat(buf_in.Load((0u + (min(((src_offset + 1u) + ubo[0u].x), ((v_15 / 4u) - 1u)) * 4u))));
    uint v_16 = 0u;
    buf_in.GetDimensions(v_16);
    float d = asfloat(buf_in.Load((0u + (min(((src_offset + 1u) + ubo[0u].x), ((v_16 / 4u) - 1u)) * 4u))));
    float sum = dot(float4(a, b, c, d), (1.0f).xxxx);
    uint v_17 = 0u;
    buf_out.GetDimensions(v_17);
    uint v_18 = (min(dst_offset, ((v_17 / 4u) - 1u)) * 4u);
    float v_19 = (sum / 4.0f);
    buf_out.Store((0u + v_18), asuint((sum - ((((v_19 < 0.0f)) ? (ceil(v_19)) : (floor(v_19))) * 4.0f))));
    float4 probabilities = (float4(a, (a * b), ((a / b) + c), sum) + max(sum, 0.0f));
    tex_out[int2(coord.xy)] = probabilities;
  }
}

vertex_main_outputs vertex_main() {
  vertex_main_outputs v_20 = {vertex_main_inner()};
  return v_20;
}

vs_main_outputs vs_main(vs_main_inputs inputs) {
  VertexInput v_21 = {inputs.VertexInput_position, inputs.VertexInput_color, inputs.VertexInput_quad_pos};
  VertexOutput v_22 = vs_main_inner(v_21);
  vs_main_outputs v_23 = {v_22.color, v_22.quad_pos, v_22.position};
  return v_23;
}

[numthreads(64, 1, 1)]
void simulate(simulate_inputs inputs) {
  simulate_inner(inputs.GlobalInvocationID);
}

[numthreads(64, 1, 1)]
void export_level(export_level_inputs inputs) {
  export_level_inner(inputs.coord);
}

