struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


RWByteAddressBuffer lightsBuffer : register(u0);
RWByteAddressBuffer tileLightId : register(u0, space1);
cbuffer cbuffer_config : register(b0, space2) {
  uint4 config[2];
};
cbuffer cbuffer_uniforms : register(b0, space3) {
  uint4 uniforms[11];
};
float4x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(uniforms[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(uniforms[((16u + start_byte_offset) / 16u)]);
  float4 v_3 = asfloat(uniforms[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_1, v_2, v_3, asfloat(uniforms[((48u + start_byte_offset) / 16u)]));
}

void main_inner(uint3 GlobalInvocationID) {
  uint index = GlobalInvocationID[0u];
  if ((index >= config[0u].x)) {
    return;
  }
  uint v_4 = (uint(index) * 32u);
  float v_5 = (asfloat(lightsBuffer.Load((4u + (uint(index) * 32u)))) - 0.10000000149011611938f);
  float v_6 = float(index);
  lightsBuffer.Store((4u + v_4), asuint((v_5 + (0.00100000004749745131f * (v_6 - (64.0f * floor((float(index) / 64.0f))))))));
  float v_7 = asfloat(lightsBuffer.Load((4u + (uint(index) * 32u))));
  if ((v_7 < asfloat(uniforms[0u].y))) {
    uint v_8 = (uint(index) * 32u);
    lightsBuffer.Store((4u + v_8), asuint(asfloat(uniforms[1u].y)));
  }
  float4x4 M = v(96u);
  float viewNear = (-(M[int(3)].z) / (-1.0f + M[int(2)].z));
  float viewFar = (-(M[int(3)].z) / (1.0f + M[int(2)].z));
  float4 lightPos = asfloat(lightsBuffer.Load4((0u + (uint(index) * 32u))));
  float4x4 v_9 = v(32u);
  lightPos = mul(lightPos, v_9);
  lightPos = (lightPos / lightPos.w);
  float lightRadius = asfloat(lightsBuffer.Load((28u + (uint(index) * 32u))));
  float4 v_10 = lightPos;
  float4 boxMin = (v_10 - float4(float3((lightRadius).xxx), 0.0f));
  float4 v_11 = lightPos;
  float4 boxMax = (v_11 + float4(float3((lightRadius).xxx), 0.0f));
  float4 frustumPlanes[6] = (float4[6])0;
  frustumPlanes[int(4)] = float4(0.0f, 0.0f, -1.0f, viewNear);
  frustumPlanes[int(5)] = float4(0.0f, 0.0f, 1.0f, -(viewFar));
  int TILE_SIZE = int(16);
  int TILE_COUNT_X = int(2);
  int TILE_COUNT_Y = int(2);
  {
    int y = int(0);
    while(true) {
      if ((y < TILE_COUNT_Y)) {
      } else {
        break;
      }
      {
        int x = int(0);
        while(true) {
          if ((x < TILE_COUNT_X)) {
          } else {
            break;
          }
          int2 tilePixel0Idx = int2((x * TILE_SIZE), (y * TILE_SIZE));
          float2 v_12 = (2.0f * float2(tilePixel0Idx));
          float2 floorCoord = ((v_12 / asfloat(uniforms[10u]).xy) - (1.0f).xx);
          int2 v_13 = tilePixel0Idx;
          float2 v_14 = (2.0f * float2((v_13 + int2((TILE_SIZE).xx))));
          float2 ceilCoord = ((v_14 / asfloat(uniforms[10u]).xy) - (1.0f).xx);
          float2 viewFloorCoord = float2((((-(viewNear) * floorCoord.x) - (M[int(2)].x * viewNear)) / M[int(0)].x), (((-(viewNear) * floorCoord.y) - (M[int(2)].y * viewNear)) / M[int(1)].y));
          float2 viewCeilCoord = float2((((-(viewNear) * ceilCoord.x) - (M[int(2)].x * viewNear)) / M[int(0)].x), (((-(viewNear) * ceilCoord.y) - (M[int(2)].y * viewNear)) / M[int(1)].y));
          frustumPlanes[int(0)] = float4(1.0f, 0.0f, (-(viewFloorCoord.x) / viewNear), 0.0f);
          frustumPlanes[int(1)] = float4(-1.0f, 0.0f, (viewCeilCoord.x / viewNear), 0.0f);
          frustumPlanes[int(2)] = float4(0.0f, 1.0f, (-(viewFloorCoord.y) / viewNear), 0.0f);
          frustumPlanes[int(3)] = float4(0.0f, -1.0f, (viewCeilCoord.y / viewNear), 0.0f);
          float dp = 0.0f;
          {
            uint i = 0u;
            while(true) {
              if ((i < 6u)) {
              } else {
                break;
              }
              float4 p = (0.0f).xxxx;
              uint v_15 = i;
              if ((frustumPlanes[v_15].x > 0.0f)) {
                p[0u] = boxMax.x;
              } else {
                p[0u] = boxMin.x;
              }
              uint v_16 = i;
              if ((frustumPlanes[v_16].y > 0.0f)) {
                p[1u] = boxMax.y;
              } else {
                p[1u] = boxMin.y;
              }
              uint v_17 = i;
              if ((frustumPlanes[v_17].z > 0.0f)) {
                p[2u] = boxMax.z;
              } else {
                p[2u] = boxMin.z;
              }
              p[3u] = 1.0f;
              float v_18 = dp;
              float4 v_19 = p;
              uint v_20 = i;
              dp = (v_18 + min(0.0f, dot(v_19, frustumPlanes[v_20])));
              {
                i = (i + 1u);
              }
              continue;
            }
          }
          if ((dp >= 0.0f)) {
            uint tileId = uint((x + (y * TILE_COUNT_X)));
            bool v_21 = false;
            if ((tileId < 0u)) {
              v_21 = true;
            } else {
              v_21 = (tileId >= config[0u].y);
            }
            if (v_21) {
              {
                x = (x + int(1));
              }
              continue;
            }
            uint v_22 = (uint(tileId) * 260u);
            uint v_23 = 0u;
            tileLightId.InterlockedAdd(uint(0u), 1u, v_23);
            uint offset = v_23;
            if ((offset >= config[1u].x)) {
              {
                x = (x + int(1));
              }
              continue;
            }
            uint v_24 = offset;
            uint v_25 = (uint(tileId) * 260u);
            tileLightId.Store(((4u + v_25) + (uint(v_24) * 4u)), GlobalInvocationID[0u]);
          }
          {
            x = (x + int(1));
          }
          continue;
        }
      }
      {
        y = (y + int(1));
      }
      continue;
    }
  }
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

