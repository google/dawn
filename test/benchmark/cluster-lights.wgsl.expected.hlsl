uint atomicAdd_1(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}

cbuffer cbuffer_camera : register(b0, space0) {
  uint4 camera[14];
};

ByteAddressBuffer clusters : register(t1, space0);

RWByteAddressBuffer clusterLights : register(u2, space0);

ByteAddressBuffer globalLights : register(t3, space0);
static const uint3 tileCount = uint3(32u, 18u, 48u);

float linearDepth(float depthSample) {
  return ((asfloat(camera[13].w) * asfloat(camera[13].z)) / mad(depthSample, (asfloat(camera[13].z) - asfloat(camera[13].w)), asfloat(camera[13].w)));
}

uint3 getTile(float4 fragCoord) {
  const float sliceScale = (float(tileCount.z) / log2((asfloat(camera[13].w) / asfloat(camera[13].z))));
  const float sliceBias = -(((float(tileCount.z) * log2(asfloat(camera[13].z))) / log2((asfloat(camera[13].w) / asfloat(camera[13].z)))));
  const uint zTile = uint(max(((log2(linearDepth(fragCoord.z)) * sliceScale) + sliceBias), 0.0f));
  return uint3(uint((fragCoord.x / (asfloat(camera[13].x) / float(tileCount.x)))), uint((fragCoord.y / (asfloat(camera[13].y) / float(tileCount.y)))), zTile);
}

uint getClusterIndex(float4 fragCoord) {
  const uint3 tile = getTile(fragCoord);
  return ((tile.x + (tile.y * tileCount.x)) + ((tile.z * tileCount.x) * tileCount.y));
}

float sqDistPointAABB(float3 tint_symbol, float3 minAABB, float3 maxAABB) {
  float sqDist = 0.0f;
  {
    [loop] for(int i = 0; (i < 3); i = (i + 1)) {
      const float v = tint_symbol[i];
      if ((v < minAABB[i])) {
        sqDist = (sqDist + ((minAABB[i] - v) * (minAABB[i] - v)));
      }
      if ((v > maxAABB[i])) {
        sqDist = (sqDist + ((v - maxAABB[i]) * (v - maxAABB[i])));
      }
    }
  }
  return sqDist;
}

struct tint_symbol_2 {
  uint3 global_id : SV_DispatchThreadID;
};

float4x4 tint_symbol_6(uint4 buffer[14], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
}

void computeMain_inner(uint3 global_id) {
  const uint tileIndex = ((global_id.x + (global_id.y * tileCount.x)) + ((global_id.z * tileCount.x) * tileCount.y));
  uint clusterLightCount = 0u;
  uint cluserLightIndices[256] = (uint[256])0;
  {
    [loop] for(uint i = 0u; (i < globalLights.Load(44u)); i = (i + 1u)) {
      const float range = asfloat(globalLights.Load(((48u + (32u * i)) + 12u)));
      bool lightInCluster = (range <= 0.0f);
      if (!(lightInCluster)) {
        const float4 lightViewPos = mul(float4(asfloat(globalLights.Load3((48u + (32u * i)))), 1.0f), tint_symbol_6(camera, 128u));
        const float sqDist = sqDistPointAABB(lightViewPos.xyz, asfloat(clusters.Load3((32u * tileIndex))), asfloat(clusters.Load3(((32u * tileIndex) + 16u))));
        lightInCluster = (sqDist <= (range * range));
      }
      if (lightInCluster) {
        cluserLightIndices[clusterLightCount] = i;
        clusterLightCount = (clusterLightCount + 1u);
      }
      if ((clusterLightCount == 256u)) {
        break;
      }
    }
  }
  uint offset = atomicAdd_1(clusterLights, 0u, clusterLightCount);
  if ((offset >= 1769472u)) {
    return;
  }
  {
    [loop] for(uint i = 0u; (i < clusterLightCount); i = (i + 1u)) {
      clusterLights.Store((221188u + (4u * (offset + i))), asuint(cluserLightIndices[i]));
    }
  }
  clusterLights.Store((4u + (8u * tileIndex)), asuint(offset));
  clusterLights.Store(((4u + (8u * tileIndex)) + 4u), asuint(clusterLightCount));
}

[numthreads(4, 2, 4)]
void computeMain(tint_symbol_2 tint_symbol_1) {
  computeMain_inner(tint_symbol_1.global_id);
  return;
}
