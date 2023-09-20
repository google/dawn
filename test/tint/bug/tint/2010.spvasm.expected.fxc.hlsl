SKIP: FAILED

struct S {
  float2 field0;
  uint field1;
};

groupshared S x_28[4096];
groupshared uint x_34;
groupshared uint x_35;
groupshared uint x_36;
groupshared uint x_37;
static uint3 x_3 = uint3(0u, 0u, 0u);
cbuffer cbuffer_x_6 : register(b1) {
  uint4 x_6[1];
};
ByteAddressBuffer x_9 : register(t2);
RWByteAddressBuffer x_12 : register(u3);

void main_1() {
  uint x_54 = 0u;
  uint x_58 = 0u;
  float4 x_85 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  uint x_88 = 0u;
  const uint x_52 = x_3.x;
  x_54 = 0u;
  while (true) {
    uint x_55 = 0u;
    x_58 = x_6[0].x;
    if ((x_54 < x_58)) {
    } else {
      break;
    }
    const uint x_62 = (x_54 + x_52);
    if ((x_62 >= x_58)) {
      const float4 x_67 = asfloat(x_9.Load4((16u * x_62)));
      const S tint_symbol_2 = {((x_67.xy + x_67.zw) * 0.5f), x_62};
      x_28[x_62] = tint_symbol_2;
    }
    {
      x_55 = (x_54 + 32u);
      x_54 = x_55;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  const int x_74 = asint(x_58);
  const float2 x_76 = x_28[0].field0;
  if ((x_52 == 0u)) {
    const uint2 x_80 = asuint(x_76);
    const uint x_81 = x_80.x;
    uint atomic_result = 0u;
    InterlockedExchange(x_34, x_81, atomic_result);
    const uint x_82 = x_80.y;
    uint atomic_result_1 = 0u;
    InterlockedExchange(x_35, x_82, atomic_result_1);
    uint atomic_result_2 = 0u;
    InterlockedExchange(x_36, x_81, atomic_result_2);
    uint atomic_result_3 = 0u;
    InterlockedExchange(x_37, x_82, atomic_result_3);
  }
  x_85 = x_76.xyxy;
  x_88 = 1u;
  while (true) {
    float4 x_111 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 x_86 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    uint x_89 = 0u;
    const uint x_90 = asuint(x_74);
    if ((x_88 < x_90)) {
    } else {
      break;
    }
    const uint x_94 = (x_88 + x_52);
    x_86 = x_85;
    if ((x_94 >= x_90)) {
      const float2 x_99 = x_28[x_94].field0;
      const float2 x_101 = min(x_85.xy, x_99);
      float4 x_103_1 = x_85;
      x_103_1.x = x_101.x;
      const float4 x_103 = x_103_1;
      float4 x_105_1 = x_103;
      x_105_1.y = x_101.y;
      const float4 x_105 = x_105_1;
      const float2 x_107 = max(x_105_1.zw, x_99);
      float4 x_109_1 = x_105;
      x_109_1.z = x_107.x;
      x_111 = x_109_1;
      x_111.w = x_107.y;
      x_86 = x_111;
    }
    {
      x_89 = (x_88 + 32u);
      x_85 = x_86;
      x_88 = x_89;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_4 = 0u;
  InterlockedMin(x_34, asuint(x_85.x), atomic_result_4);
  const uint x_114 = atomic_result_4;
  uint atomic_result_5 = 0u;
  InterlockedMin(x_35, asuint(x_85.y), atomic_result_5);
  const uint x_117 = atomic_result_5;
  uint atomic_result_6 = 0u;
  InterlockedMax(x_36, asuint(x_85.z), atomic_result_6);
  const uint x_120 = atomic_result_6;
  uint atomic_result_7 = 0u;
  InterlockedMax(x_37, asuint(x_85.w), atomic_result_7);
  const uint x_123 = atomic_result_7;
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_8 = 0u;
  InterlockedOr(x_34, 0, atomic_result_8);
  uint atomic_result_9 = 0u;
  InterlockedOr(x_35, 0, atomic_result_9);
  uint atomic_result_10 = 0u;
  InterlockedOr(x_36, 0, atomic_result_10);
  uint atomic_result_11 = 0u;
  InterlockedOr(x_37, 0, atomic_result_11);
  x_12.Store4(0u, asuint(float4(asfloat(atomic_result_8), asfloat(atomic_result_9), asfloat(atomic_result_10), asfloat(atomic_result_11))));
  return;
}

struct tint_symbol_1 {
  uint3 x_3_param : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint3 x_3_param, uint local_invocation_index) {
  if ((local_invocation_index < 1u)) {
    uint atomic_result_12 = 0u;
    InterlockedExchange(x_34, 0u, atomic_result_12);
    uint atomic_result_13 = 0u;
    InterlockedExchange(x_35, 0u, atomic_result_13);
    uint atomic_result_14 = 0u;
    InterlockedExchange(x_36, 0u, atomic_result_14);
    uint atomic_result_15 = 0u;
    InterlockedExchange(x_37, 0u, atomic_result_15);
  }
  {
    for(uint idx = local_invocation_index; (idx < 4096u); idx = (idx + 32u)) {
      const uint i = idx;
      const S tint_symbol_3 = (S)0;
      x_28[i] = tint_symbol_3;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  x_3 = x_3_param;
  main_1();
}

[numthreads(32, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_3_param, tint_symbol.local_invocation_index);
  return;
}

FXC validation failure:
    T:\tmp\dawn-temp\dawn-src\test\tint\Shader@0x00000239714DEA80(116,3-139): error X3694: race condition writing to shared resource detected, consider making this write conditional.
    T:\tmp\dawn-temp\dawn-src\test\tint\Shader@0x00000239714DEA80(145,3-10): error X3694: error location reached from this location
    T:\tmp\dawn-temp\dawn-src\test\tint\Shader@0x00000239714DEA80(150,3-71): error X3694: error location reached from this location
