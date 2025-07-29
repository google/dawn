struct main_inputs {
  uint3 lid : SV_GroupThreadID;
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_U : register(b3) {
  uint4 U[4];
};
RWTexture2D<uint4> dst_image2d : register(u1);
Texture2D<float4> src_image2d : register(t2);
groupshared uint4 outputs[8][32];
uint4 tint_v4f32_to_v4u32(float4 value) {
  return uint4(clamp(value, (0.0f).xxxx, (4294967040.0f).xxxx));
}

void main_inner(uint3 lid, uint tint_local_index) {
  if ((tint_local_index < 256u)) {
    outputs[(tint_local_index / 32u)][(tint_local_index % 32u)] = (0u).xxxx;
  }
  GroupMemoryBarrierWithGroupSync();
  int init = int(lid.z);
  {
    int S = init;
    while((S < asint(U[3u].x))) {
      {
        S = (S + int(8));
      }
      continue;
    }
  }
  {
    int s_group = int(0);
    while((s_group < asint(U[3u].z))) {
      outputs[lid.z][lid.x] = tint_v4f32_to_v4u32(src_image2d.Load(int3(int2(uint2((uint(asint(U[3u].x))).xx)), int(0))));
      GroupMemoryBarrierWithGroupSync();
      uint4 result = outputs[lid.z][lid.x];
      uint2 v = uint2((uint(asint(U[3u].x))).xx);
      dst_image2d[v] = result;
      {
        s_group = (s_group + int(8));
      }
      continue;
    }
  }
}

[numthreads(32, 1, 8)]
void main(main_inputs inputs) {
  main_inner(inputs.lid, inputs.tint_local_index);
}

