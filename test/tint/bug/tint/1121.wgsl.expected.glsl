#version 310 es

struct LightData {
  vec4 position;
  vec3 color;
  float radius;
};

layout(binding = 0, std430) buffer LightsBuffer_ssbo {
  LightData lights[];
} lightsBuffer;

struct TileLightIdData {
  uint count;
  uint lightId[64];
};

layout(binding = 0, std430) buffer Tiles_ssbo {
  TileLightIdData data[4];
} tileLightId;

layout(binding = 0, std140) uniform Config_ubo {
  uint numLights;
  uint numTiles;
  uint tileCountX;
  uint tileCountY;
  uint numTileLightSlot;
  uint tileSize;
  uint pad;
  uint pad_1;
} config;

layout(binding = 0, std140) uniform Uniforms_ubo {
  vec4 tint_symbol;
  vec4 tint_symbol_1;
  mat4 viewMatrix;
  mat4 projectionMatrix;
  vec4 fullScreenSize;
} uniforms;

void tint_symbol_2(uvec3 GlobalInvocationID) {
  uint index = GlobalInvocationID.x;
  if ((index >= config.numLights)) {
    return;
  }
  lightsBuffer.lights[index].position.y = ((lightsBuffer.lights[index].position.y - 0.100000001f) + (0.001f * (float(index) - (64.0f * floor((float(index) / 64.0f))))));
  if ((lightsBuffer.lights[index].position.y < uniforms.tint_symbol.y)) {
    lightsBuffer.lights[index].position.y = uniforms.tint_symbol_1.y;
  }
  mat4 M = uniforms.projectionMatrix;
  float viewNear = (-(M[3][2]) / (-1.0f + M[2][2]));
  float viewFar = (-(M[3][2]) / (1.0f + M[2][2]));
  vec4 lightPos = lightsBuffer.lights[index].position;
  lightPos = (uniforms.viewMatrix * lightPos);
  lightPos = (lightPos / lightPos.w);
  float lightRadius = lightsBuffer.lights[index].radius;
  vec4 boxMin = (lightPos - vec4(vec3(lightRadius), 0.0f));
  vec4 boxMax = (lightPos + vec4(vec3(lightRadius), 0.0f));
  vec4 frustumPlanes[6] = vec4[6](vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  frustumPlanes[4] = vec4(0.0f, 0.0f, -1.0f, viewNear);
  frustumPlanes[5] = vec4(0.0f, 0.0f, 1.0f, -(viewFar));
  int TILE_SIZE = 16;
  int TILE_COUNT_X = 2;
  {
    for(int y_1 = 0; (y_1 < 2); y_1 = (y_1 + 1)) {
      {
        for(int x_1 = 0; (x_1 < TILE_COUNT_X); x_1 = (x_1 + 1)) {
          ivec2 tilePixel0Idx = ivec2((x_1 * TILE_SIZE), (y_1 * TILE_SIZE));
          vec2 floorCoord = (((2.0f * vec2(tilePixel0Idx)) / uniforms.fullScreenSize.xy) - vec2(1.0f));
          vec2 ceilCoord = (((2.0f * vec2((tilePixel0Idx + ivec2(TILE_SIZE)))) / uniforms.fullScreenSize.xy) - vec2(1.0f));
          vec2 viewFloorCoord = vec2((((-(viewNear) * floorCoord.x) - (M[2][0] * viewNear)) / M[0][0]), (((-(viewNear) * floorCoord.y) - (M[2][1] * viewNear)) / M[1][1]));
          vec2 viewCeilCoord = vec2((((-(viewNear) * ceilCoord.x) - (M[2][0] * viewNear)) / M[0][0]), (((-(viewNear) * ceilCoord.y) - (M[2][1] * viewNear)) / M[1][1]));
          frustumPlanes[0] = vec4(1.0f, 0.0f, (-(viewFloorCoord.x) / viewNear), 0.0f);
          frustumPlanes[1] = vec4(-1.0f, 0.0f, (viewCeilCoord.x / viewNear), 0.0f);
          frustumPlanes[2] = vec4(0.0f, 1.0f, (-(viewFloorCoord.y) / viewNear), 0.0f);
          frustumPlanes[3] = vec4(0.0f, -1.0f, (viewCeilCoord.y / viewNear), 0.0f);
          float dp = 0.0f;
          {
            for(uint i = 0u; (i < 6u); i = (i + 1u)) {
              vec4 p = vec4(0.0f, 0.0f, 0.0f, 0.0f);
              if ((frustumPlanes[i].x > 0.0f)) {
                p.x = boxMax.x;
              } else {
                p.x = boxMin.x;
              }
              if ((frustumPlanes[i].y > 0.0f)) {
                p.y = boxMax.y;
              } else {
                p.y = boxMin.y;
              }
              if ((frustumPlanes[i].z > 0.0f)) {
                p.z = boxMax.z;
              } else {
                p.z = boxMin.z;
              }
              p.w = 1.0f;
              dp = (dp + min(0.0f, dot(p, frustumPlanes[i])));
            }
          }
          if ((dp >= 0.0f)) {
            uint tileId = uint((x_1 + (y_1 * TILE_COUNT_X)));
            bool tint_tmp = (tileId < 0u);
            if (!tint_tmp) {
              tint_tmp = (tileId >= config.numTiles);
            }
            if ((tint_tmp)) {
              continue;
            }
            uint offset = atomicAdd(tileLightId.data[tileId].count, 1u);
            if ((offset >= config.numTileLightSlot)) {
              continue;
            }
            tileLightId.data[tileId].lightId[offset] = GlobalInvocationID.x;
          }
        }
      }
    }
  }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2(gl_GlobalInvocationID);
  return;
}
