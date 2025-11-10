#version 310 es


struct Particle {
  vec2 pos;
  vec2 vel;
};

struct Particles {
  Particle particles[5];
};

layout(binding = 0, std140)
uniform params_block_1_ubo {
  uvec4 inner[2];
} v;
layout(binding = 1, std430)
buffer particlesA_block_1_ssbo {
  Particles inner;
} v_1;
layout(binding = 2, std430)
buffer particlesB_block_1_ssbo {
  Particles inner;
} v_2;
void comp_main_inner(uvec3 v_3) {
  uint index = v_3.x;
  if ((index >= 5u)) {
    return;
  }
  uint v_4 = min(index, 4u);
  vec2 vPos = v_1.inner.particles[v_4].pos;
  uint v_5 = min(index, 4u);
  vec2 vVel = v_1.inner.particles[v_5].vel;
  vec2 cMass = vec2(0.0f);
  vec2 cVel = vec2(0.0f);
  vec2 colVel = vec2(0.0f);
  int cMassCount = 0;
  int cVelCount = 0;
  vec2 pos = vec2(0.0f);
  vec2 vel = vec2(0.0f);
  {
    uint i = 0u;
    while(true) {
      if ((i < 5u)) {
      } else {
        break;
      }
      if ((i == index)) {
        {
          i = (i + 1u);
        }
        continue;
      }
      uint v_6 = i;
      pos = v_1.inner.particles[v_6].pos.xy;
      uint v_7 = i;
      vel = v_1.inner.particles[v_7].vel.xy;
      uvec4 v_8 = v.inner[0u];
      if ((distance(pos, vPos) < uintBitsToFloat(v_8.y))) {
        cMass = (cMass + pos);
        uint v_9 = uint(cMassCount);
        cMassCount = int((v_9 + uint(1)));
      }
      uvec4 v_10 = v.inner[0u];
      if ((distance(pos, vPos) < uintBitsToFloat(v_10.z))) {
        colVel = (colVel - (pos - vPos));
      }
      uvec4 v_11 = v.inner[0u];
      if ((distance(pos, vPos) < uintBitsToFloat(v_11.w))) {
        cVel = (cVel + vel);
        uint v_12 = uint(cVelCount);
        cVelCount = int((v_12 + uint(1)));
      }
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  if ((cMassCount > 0)) {
    vec2 v_13 = cMass;
    float v_14 = float(cMassCount);
    vec2 v_15 = (v_13 / vec2(v_14, float(cMassCount)));
    cMass = (v_15 - vPos);
  }
  if ((cVelCount > 0)) {
    vec2 v_16 = cVel;
    float v_17 = float(cVelCount);
    cVel = (v_16 / vec2(v_17, float(cVelCount)));
  }
  uvec4 v_18 = v.inner[1u];
  uvec4 v_19 = v.inner[1u];
  uvec4 v_20 = v.inner[1u];
  vVel = (((vVel + (cMass * uintBitsToFloat(v_18.x))) + (colVel * uintBitsToFloat(v_19.y))) + (cVel * uintBitsToFloat(v_20.z)));
  vVel = (normalize(vVel) * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  uvec4 v_21 = v.inner[0u];
  vPos = (vPos + (vVel * uintBitsToFloat(v_21.x)));
  if ((vPos.x < -1.0f)) {
    vPos.x = 1.0f;
  }
  if ((vPos.x > 1.0f)) {
    vPos.x = -1.0f;
  }
  if ((vPos.y < -1.0f)) {
    vPos.y = 1.0f;
  }
  if ((vPos.y > 1.0f)) {
    vPos.y = -1.0f;
  }
  uint v_22 = min(index, 4u);
  v_2.inner.particles[v_22].pos = vPos;
  uint v_23 = min(index, 4u);
  v_2.inner.particles[v_23].vel = vVel;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  comp_main_inner(gl_GlobalInvocationID);
}
