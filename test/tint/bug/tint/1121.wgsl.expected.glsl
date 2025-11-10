#version 310 es


struct LightData {
  vec4 position;
  vec3 color;
  float radius;
};

struct TileLightIdData {
  uint count;
  uint lightId[64];
};

struct Tiles {
  TileLightIdData data[4];
};

layout(binding = 0, std430)
buffer LightsBuffer_1_ssbo {
  LightData lights[];
} lightsBuffer;
layout(binding = 3, std430)
buffer tileLightId_block_1_ssbo {
  Tiles inner;
} v;
layout(binding = 1, std140)
uniform config_block_1_ubo {
  uvec4 inner[2];
} v_1;
layout(binding = 2, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[11];
} v_2;
mat4 v_3(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v_2.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_2.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v_2.inner[((48u + start_byte_offset) / 16u)]));
}
void main_inner(uvec3 GlobalInvocationID) {
  uint index = GlobalInvocationID.x;
  uvec4 v_4 = v_1.inner[0u];
  if ((index >= v_4.x)) {
    return;
  }
  uint v_5 = index;
  uint v_6 = min(v_5, (uint(lightsBuffer.lights.length()) - 1u));
  uint v_7 = index;
  uint v_8 = min(v_7, (uint(lightsBuffer.lights.length()) - 1u));
  float v_9 = (lightsBuffer.lights[v_8].position.y - 0.10000000149011611938f);
  float v_10 = float(index);
  lightsBuffer.lights[v_6].position.y = (v_9 + (0.00100000004749745131f * (v_10 - (64.0f * floor((float(index) / 64.0f))))));
  uint v_11 = index;
  uint v_12 = min(v_11, (uint(lightsBuffer.lights.length()) - 1u));
  uvec4 v_13 = v_2.inner[0u];
  if ((lightsBuffer.lights[v_12].position.y < uintBitsToFloat(v_13.y))) {
    uint v_14 = index;
    uint v_15 = min(v_14, (uint(lightsBuffer.lights.length()) - 1u));
    uvec4 v_16 = v_2.inner[1u];
    lightsBuffer.lights[v_15].position.y = uintBitsToFloat(v_16.y);
  }
  mat4 M = v_3(96u);
  float viewNear = (-(M[3u].z) / (-1.0f + M[2u].z));
  float viewFar = (-(M[3u].z) / (1.0f + M[2u].z));
  uint v_17 = index;
  uint v_18 = min(v_17, (uint(lightsBuffer.lights.length()) - 1u));
  vec4 lightPos = lightsBuffer.lights[v_18].position;
  mat4 v_19 = v_3(32u);
  lightPos = (v_19 * lightPos);
  lightPos = (lightPos / lightPos.w);
  uint v_20 = index;
  uint v_21 = min(v_20, (uint(lightsBuffer.lights.length()) - 1u));
  float lightRadius = lightsBuffer.lights[v_21].radius;
  vec4 v_22 = lightPos;
  vec4 boxMin = (v_22 - vec4(vec3(lightRadius), 0.0f));
  vec4 v_23 = lightPos;
  vec4 boxMax = (v_23 + vec4(vec3(lightRadius), 0.0f));
  vec4 frustumPlanes[6] = vec4[6](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  frustumPlanes[4u] = vec4(0.0f, 0.0f, -1.0f, viewNear);
  frustumPlanes[5u] = vec4(0.0f, 0.0f, 1.0f, -(viewFar));
  int TILE_SIZE = 16;
  int TILE_COUNT_X = 2;
  int TILE_COUNT_Y = 2;
  {
    int y = 0;
    while(true) {
      if ((y < TILE_COUNT_Y)) {
      } else {
        break;
      }
      {
        int x = 0;
        while(true) {
          if ((x < TILE_COUNT_X)) {
          } else {
            break;
          }
          uint v_24 = uint(x);
          int v_25 = int((v_24 * uint(TILE_SIZE)));
          uint v_26 = uint(y);
          ivec2 tilePixel0Idx = ivec2(v_25, int((v_26 * uint(TILE_SIZE))));
          vec2 v_27 = (2.0f * vec2(tilePixel0Idx));
          vec2 floorCoord = ((v_27 / uintBitsToFloat(v_2.inner[10u]).xy) - vec2(1.0f));
          ivec2 v_28 = tilePixel0Idx;
          ivec2 v_29 = ivec2(TILE_SIZE);
          uvec2 v_30 = uvec2(v_28);
          vec2 v_31 = (2.0f * vec2(ivec2((v_30 + uvec2(v_29)))));
          vec2 ceilCoord = ((v_31 / uintBitsToFloat(v_2.inner[10u]).xy) - vec2(1.0f));
          vec2 viewFloorCoord = vec2((((-(viewNear) * floorCoord.x) - (M[2u].x * viewNear)) / M[0u].x), (((-(viewNear) * floorCoord.y) - (M[2u].y * viewNear)) / M[1u].y));
          vec2 viewCeilCoord = vec2((((-(viewNear) * ceilCoord.x) - (M[2u].x * viewNear)) / M[0u].x), (((-(viewNear) * ceilCoord.y) - (M[2u].y * viewNear)) / M[1u].y));
          frustumPlanes[0u] = vec4(1.0f, 0.0f, (-(viewFloorCoord.x) / viewNear), 0.0f);
          frustumPlanes[1u] = vec4(-1.0f, 0.0f, (viewCeilCoord.x / viewNear), 0.0f);
          frustumPlanes[2u] = vec4(0.0f, 1.0f, (-(viewFloorCoord.y) / viewNear), 0.0f);
          frustumPlanes[3u] = vec4(0.0f, -1.0f, (viewCeilCoord.y / viewNear), 0.0f);
          float dp = 0.0f;
          {
            uint i = 0u;
            while(true) {
              if ((i < 6u)) {
              } else {
                break;
              }
              vec4 p = vec4(0.0f);
              uint v_32 = i;
              if ((frustumPlanes[v_32].x > 0.0f)) {
                p.x = boxMax.x;
              } else {
                p.x = boxMin.x;
              }
              uint v_33 = i;
              if ((frustumPlanes[v_33].y > 0.0f)) {
                p.y = boxMax.y;
              } else {
                p.y = boxMin.y;
              }
              uint v_34 = i;
              if ((frustumPlanes[v_34].z > 0.0f)) {
                p.z = boxMax.z;
              } else {
                p.z = boxMin.z;
              }
              p.w = 1.0f;
              float v_35 = dp;
              vec4 v_36 = p;
              uint v_37 = i;
              dp = (v_35 + min(0.0f, dot(v_36, frustumPlanes[v_37])));
              {
                i = (i + 1u);
              }
              continue;
            }
          }
          if ((dp >= 0.0f)) {
            int v_38 = x;
            uint v_39 = uint(y);
            int v_40 = int((v_39 * uint(TILE_COUNT_X)));
            uint v_41 = uint(v_38);
            uint tileId = uint(int((v_41 + uint(v_40))));
            bool v_42 = false;
            if ((tileId < 0u)) {
              v_42 = true;
            } else {
              uvec4 v_43 = v_1.inner[0u];
              v_42 = (tileId >= v_43.y);
            }
            if (v_42) {
              {
                uint v_44 = uint(x);
                x = int((v_44 + uint(1)));
              }
              continue;
            }
            uint v_45 = min(tileId, 3u);
            uint offset = atomicAdd(v.inner.data[v_45].count, 1u);
            uvec4 v_46 = v_1.inner[1u];
            if ((offset >= v_46.x)) {
              {
                uint v_44 = uint(x);
                x = int((v_44 + uint(1)));
              }
              continue;
            }
            uint v_47 = min(tileId, 3u);
            uint v_48 = min(offset, 63u);
            v.inner.data[v_47].lightId[v_48] = GlobalInvocationID.x;
          }
          {
            uint v_44 = uint(x);
            x = int((v_44 + uint(1)));
          }
          continue;
        }
      }
      {
        uint v_49 = uint(y);
        y = int((v_49 + uint(1)));
      }
      continue;
    }
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
