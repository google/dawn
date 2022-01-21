#version 310 es
precision mediump float;

struct Camera {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  vec3 position;
  float time;
  vec2 outputSize;
  float zNear;
  float zFar;
};

layout (binding = 0) uniform Camera_1 {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  vec3 position;
  float time;
  vec2 outputSize;
  float zNear;
  float zFar;
} camera;

struct ClusterBounds {
  vec3 minAABB;
  vec3 maxAABB;
};
struct Clusters {
  ClusterBounds bounds[27648];
};

layout (binding = 1) buffer Clusters_1 {
  ClusterBounds bounds[27648];
} clusters;

struct ClusterLights {
  uint offset;
  uint count;
};
struct ClusterLightGroup {
  uint offset;
  ClusterLights lights[27648];
  uint indices[1769472];
};

layout (binding = 2) buffer ClusterLightGroup_1 {
  uint offset;
  ClusterLights lights[27648];
  uint indices[1769472];
} clusterLights;

struct Light {
  vec3 position;
  float range;
  vec3 color;
  float intensity;
};

layout (binding = 3) buffer GlobalLights_1 {
  vec3 ambient;
  vec3 dirColor;
  float dirIntensity;
  vec3 dirDirection;
  uint lightCount;
  Light lights[];
} globalLights;
const uvec3 tileCount = uvec3(32u, 18u, 48u);

float sqDistPointAABB(vec3 point, vec3 minAABB, vec3 maxAABB) {
  float sqDist = 0.0f;
  {
    for(int i = 0; (i < 3); i = (i + 1)) {
      float v = point[i];
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

struct tint_symbol_1 {
  uvec3 global_id;
};

void computeMain_inner(uvec3 global_id) {
  uint tileIndex = ((global_id.x + (global_id.y * tileCount.x)) + ((global_id.z * tileCount.x) * tileCount.y));
  uint clusterLightCount = 0u;
  uint cluserLightIndices[256] = uint[256](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
  {
    for(uint i = 0u; (i < globalLights.lightCount); i = (i + 1u)) {
      float range = globalLights.lights[i].range;
      bool lightInCluster = (range <= 0.0f);
      if (!(lightInCluster)) {
        vec4 lightViewPos = (camera.view * vec4(globalLights.lights[i].position, 1.0f));
        float sqDist = sqDistPointAABB(lightViewPos.xyz, clusters.bounds[tileIndex].minAABB, clusters.bounds[tileIndex].maxAABB);
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
  uint offset = atomicAdd(clusterLights.offset, clusterLightCount);
  if ((offset >= 1769472u)) {
    return;
  }
  {
    for(uint i = 0u; (i < clusterLightCount); i = (i + 1u)) {
      clusterLights.indices[(offset + i)] = cluserLightIndices[i];
    }
  }
  clusterLights.lights[tileIndex].offset = offset;
  clusterLights.lights[tileIndex].count = clusterLightCount;
}

layout(local_size_x = 4, local_size_y = 2, local_size_z = 4) in;
void computeMain(tint_symbol_1 tint_symbol) {
  computeMain_inner(tint_symbol.global_id);
  return;
}
void main() {
  tint_symbol_1 inputs;
  inputs.global_id = gl_GlobalInvocationID;
  computeMain(inputs);
}


