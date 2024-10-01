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

struct Config {
  uint numLights;
  uint numTiles;
  uint tileCountX;
  uint tileCountY;
  uint numTileLightSlot;
  uint tileSize;
};

struct Uniforms {
  vec4 tint_symbol;
  vec4 tint_symbol_1;
  mat4 viewMatrix;
  mat4 projectionMatrix;
  vec4 fullScreenSize;
};

layout(binding = 0, std430)
buffer LightsBuffer_1_ssbo {
  LightData lights[];
} lightsBuffer;
layout(binding = 0, std430)
buffer tint_symbol_4_1_ssbo {
  Tiles tint_symbol_3;
} v;
layout(binding = 0, std140)
uniform tint_symbol_6_1_ubo {
  Config tint_symbol_5;
} v_1;
layout(binding = 0, std140)
uniform tint_symbol_8_1_ubo {
  Uniforms tint_symbol_7;
} v_2;
void tint_symbol_2_inner(uvec3 GlobalInvocationID) {
  uint index = GlobalInvocationID[0u];
  if ((index >= v_1.tint_symbol_5.numLights)) {
    return;
  }
  uint v_3 = index;
  uint v_4 = index;
  float v_5 = (lightsBuffer.lights[v_4].position.y - 0.10000000149011611938f);
  float v_6 = float(index);
  lightsBuffer.lights[v_3].position[1u] = (v_5 + (0.00100000004749745131f * (v_6 - (64.0f * floor((float(index) / 64.0f))))));
  uint v_7 = index;
  if ((lightsBuffer.lights[v_7].position.y < v_2.tint_symbol_7.tint_symbol.y)) {
    uint v_8 = index;
    lightsBuffer.lights[v_8].position[1u] = v_2.tint_symbol_7.tint_symbol_1.y;
  }
  mat4 M = v_2.tint_symbol_7.projectionMatrix;
  float viewNear = (-(M[3].z) / (-1.0f + M[2].z));
  float viewFar = (-(M[3].z) / (1.0f + M[2].z));
  uint v_9 = index;
  vec4 lightPos = lightsBuffer.lights[v_9].position;
  lightPos = (v_2.tint_symbol_7.viewMatrix * lightPos);
  lightPos = (lightPos / lightPos.w);
  uint v_10 = index;
  float lightRadius = lightsBuffer.lights[v_10].radius;
  vec4 v_11 = lightPos;
  vec4 boxMin = (v_11 - vec4(vec3(lightRadius), 0.0f));
  vec4 v_12 = lightPos;
  vec4 boxMax = (v_12 + vec4(vec3(lightRadius), 0.0f));
  vec4 frustumPlanes[6] = vec4[6](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  frustumPlanes[4] = vec4(0.0f, 0.0f, -1.0f, viewNear);
  frustumPlanes[5] = vec4(0.0f, 0.0f, 1.0f, -(viewFar));
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
          ivec2 tilePixel0Idx = ivec2((x * TILE_SIZE), (y * TILE_SIZE));
          vec2 v_13 = (2.0f * vec2(tilePixel0Idx));
          vec2 floorCoord = ((v_13 / v_2.tint_symbol_7.fullScreenSize.xy) - vec2(1.0f));
          ivec2 v_14 = tilePixel0Idx;
          vec2 v_15 = (2.0f * vec2((v_14 + ivec2(TILE_SIZE))));
          vec2 ceilCoord = ((v_15 / v_2.tint_symbol_7.fullScreenSize.xy) - vec2(1.0f));
          vec2 viewFloorCoord = vec2((((-(viewNear) * floorCoord.x) - (M[2].x * viewNear)) / M[0].x), (((-(viewNear) * floorCoord.y) - (M[2].y * viewNear)) / M[1].y));
          vec2 viewCeilCoord = vec2((((-(viewNear) * ceilCoord.x) - (M[2].x * viewNear)) / M[0].x), (((-(viewNear) * ceilCoord.y) - (M[2].y * viewNear)) / M[1].y));
          frustumPlanes[0] = vec4(1.0f, 0.0f, (-(viewFloorCoord.x) / viewNear), 0.0f);
          frustumPlanes[1] = vec4(-1.0f, 0.0f, (viewCeilCoord.x / viewNear), 0.0f);
          frustumPlanes[2] = vec4(0.0f, 1.0f, (-(viewFloorCoord.y) / viewNear), 0.0f);
          frustumPlanes[3] = vec4(0.0f, -1.0f, (viewCeilCoord.y / viewNear), 0.0f);
          float dp = 0.0f;
          {
            uint i = 0u;
            while(true) {
              if ((i < 6u)) {
              } else {
                break;
              }
              vec4 p = vec4(0.0f);
              uint v_16 = i;
              if ((frustumPlanes[v_16].x > 0.0f)) {
                p[0u] = boxMax.x;
              } else {
                p[0u] = boxMin.x;
              }
              uint v_17 = i;
              if ((frustumPlanes[v_17].y > 0.0f)) {
                p[1u] = boxMax.y;
              } else {
                p[1u] = boxMin.y;
              }
              uint v_18 = i;
              if ((frustumPlanes[v_18].z > 0.0f)) {
                p[2u] = boxMax.z;
              } else {
                p[2u] = boxMin.z;
              }
              p[3u] = 1.0f;
              float v_19 = dp;
              vec4 v_20 = p;
              uint v_21 = i;
              dp = (v_19 + min(0.0f, dot(v_20, frustumPlanes[v_21])));
              {
                i = (i + 1u);
              }
              continue;
            }
          }
          if ((dp >= 0.0f)) {
            uint tileId = uint((x + (y * TILE_COUNT_X)));
            bool v_22 = false;
            if ((tileId < 0u)) {
              v_22 = true;
            } else {
              v_22 = (tileId >= v_1.tint_symbol_5.numTiles);
            }
            if (v_22) {
              {
                x = (x + 1);
              }
              continue;
            }
            uint v_23 = tileId;
            uint offset = atomicAdd(v.tint_symbol_3.data[v_23].count, 1u);
            if ((offset >= v_1.tint_symbol_5.numTileLightSlot)) {
              {
                x = (x + 1);
              }
              continue;
            }
            uint v_24 = tileId;
            uint v_25 = offset;
            v.tint_symbol_3.data[v_24].lightId[v_25] = GlobalInvocationID[0u];
          }
          {
            x = (x + 1);
          }
          continue;
        }
      }
      {
        y = (y + 1);
      }
      continue;
    }
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2_inner(gl_GlobalInvocationID);
}
