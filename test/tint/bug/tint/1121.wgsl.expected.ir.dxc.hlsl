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
  return float4x4(asfloat(uniforms[(start_byte_offset / 16u)]), asfloat(uniforms[((16u + start_byte_offset) / 16u)]), asfloat(uniforms[((32u + start_byte_offset) / 16u)]), asfloat(uniforms[((48u + start_byte_offset) / 16u)]));
}

void main_inner(uint3 GlobalInvocationID) {
  uint index = GlobalInvocationID.x;
  if ((index >= config[0u].x)) {
    return;
  }
  uint v_1 = (uint(index) * 32u);
  float v_2 = (asfloat(lightsBuffer.Load((4u + (uint(index) * 32u)))) - 0.10000000149011611938f);
  float v_3 = float(index);
  lightsBuffer.Store((4u + v_1), asuint((v_2 + (0.00100000004749745131f * (v_3 - (64.0f * floor((float(index) / 64.0f))))))));
  float v_4 = asfloat(lightsBuffer.Load((4u + (uint(index) * 32u))));
  if ((v_4 < asfloat(uniforms[0u].y))) {
    uint v_5 = (uint(index) * 32u);
    lightsBuffer.Store((4u + v_5), asuint(asfloat(uniforms[1u].y)));
  }
  float4x4 M = v(96u);
  float viewNear = (-(M[int(3)].z) / (-1.0f + M[int(2)].z));
  float viewFar = (-(M[int(3)].z) / (1.0f + M[int(2)].z));
  float4 lightPos = asfloat(lightsBuffer.Load4((0u + (uint(index) * 32u))));
  float4x4 v_6 = v(32u);
  lightPos = mul(lightPos, v_6);
  lightPos = (lightPos / lightPos.w);
  float lightRadius = asfloat(lightsBuffer.Load((28u + (uint(index) * 32u))));
  float4 v_7 = lightPos;
  float4 boxMin = (v_7 - float4(float3((lightRadius).xxx), 0.0f));
  float4 v_8 = lightPos;
  float4 boxMax = (v_8 + float4(float3((lightRadius).xxx), 0.0f));
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
          float2 v_9 = (2.0f * float2(tilePixel0Idx));
          float2 floorCoord = ((v_9 / asfloat(uniforms[10u]).xy) - (1.0f).xx);
          int2 v_10 = tilePixel0Idx;
          float2 v_11 = (2.0f * float2((v_10 + int2((TILE_SIZE).xx))));
          float2 ceilCoord = ((v_11 / asfloat(uniforms[10u]).xy) - (1.0f).xx);
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
              uint v_12 = i;
              if ((frustumPlanes[v_12].x > 0.0f)) {
                p.x = boxMax.x;
              } else {
                p.x = boxMin.x;
              }
              uint v_13 = i;
              if ((frustumPlanes[v_13].y > 0.0f)) {
                p.y = boxMax.y;
              } else {
                p.y = boxMin.y;
              }
              uint v_14 = i;
              if ((frustumPlanes[v_14].z > 0.0f)) {
                p.z = boxMax.z;
              } else {
                p.z = boxMin.z;
              }
              p.w = 1.0f;
              float v_15 = dp;
              float4 v_16 = p;
              uint v_17 = i;
              dp = (v_15 + min(0.0f, dot(v_16, frustumPlanes[v_17])));
              {
                i = (i + 1u);
              }
              continue;
            }
          }
          if ((dp >= 0.0f)) {
            uint tileId = uint((x + (y * TILE_COUNT_X)));
            bool v_18 = false;
            if ((tileId < 0u)) {
              v_18 = true;
            } else {
              v_18 = (tileId >= config[0u].y);
            }
            if (v_18) {
              {
                x = (x + int(1));
              }
              continue;
            }
            uint v_19 = 0u;
            tileLightId.InterlockedAdd(uint((0u + (uint(tileId) * 260u))), 1u, v_19);
            uint offset = v_19;
            if ((offset >= config[1u].x)) {
              {
                x = (x + int(1));
              }
              continue;
            }
            uint v_20 = offset;
            uint v_21 = (uint(tileId) * 260u);
            tileLightId.Store(((4u + v_21) + (uint(v_20) * 4u)), GlobalInvocationID.x);
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

