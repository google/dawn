#version 310 es

layout(location = 0) in vec2 vert_main_loc0_Input;
layout(location = 1) in vec2 vert_main_loc1_Input;
layout(location = 2) in vec2 vert_main_loc2_Input;
vec4 vert_main_inner(vec2 a_particlePos, vec2 a_particleVel, vec2 a_pos) {
  float angle = -(atan(a_particleVel[0u], a_particleVel[1u]));
  float v = (a_pos[0u] * cos(angle));
  float v_1 = (v - (a_pos[1u] * sin(angle)));
  float v_2 = (a_pos[0u] * sin(angle));
  vec2 pos = vec2(v_1, (v_2 + (a_pos[1u] * cos(angle))));
  return vec4((pos + a_particlePos), 0.0f, 1.0f);
}
void main() {
  gl_Position = vert_main_inner(vert_main_loc0_Input, vert_main_loc1_Input, vert_main_loc2_Input);
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 frag_main_loc0_Output;
vec4 frag_main_inner() {
  return vec4(1.0f);
}
void main() {
  frag_main_loc0_Output = frag_main_inner();
}
#version 310 es


struct SimParams {
  float deltaT;
  float rule1Distance;
  float rule2Distance;
  float rule3Distance;
  float rule1Scale;
  float rule2Scale;
  float rule3Scale;
};

struct Particle {
  vec2 pos;
  vec2 vel;
};

struct Particles {
  Particle particles[5];
};

layout(binding = 0, std140)
uniform params_block_1_ubo {
  SimParams inner;
} v;
layout(binding = 1, std430)
buffer particlesA_block_1_ssbo {
  Particles inner;
} v_1;
layout(binding = 2, std430)
buffer particlesB_block_1_ssbo {
  Particles inner;
} v_2;
void comp_main_inner(uvec3 tint_symbol) {
  uint index = tint_symbol[0u];
  if ((index >= 5u)) {
    return;
  }
  uint v_3 = index;
  vec2 vPos = v_1.inner.particles[v_3].pos;
  uint v_4 = index;
  vec2 vVel = v_1.inner.particles[v_4].vel;
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
      uint v_5 = i;
      pos = v_1.inner.particles[v_5].pos.xy;
      uint v_6 = i;
      vel = v_1.inner.particles[v_6].vel.xy;
      float v_7 = distance(pos, vPos);
      if ((v_7 < v.inner.rule1Distance)) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      float v_8 = distance(pos, vPos);
      if ((v_8 < v.inner.rule2Distance)) {
        colVel = (colVel - (pos - vPos));
      }
      float v_9 = distance(pos, vPos);
      if ((v_9 < v.inner.rule3Distance)) {
        cVel = (cVel + vel);
        cVelCount = (cVelCount + 1);
      }
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  if ((cMassCount > 0)) {
    vec2 v_10 = cMass;
    float v_11 = float(cMassCount);
    vec2 v_12 = (v_10 / vec2(v_11, float(cMassCount)));
    cMass = (v_12 - vPos);
  }
  if ((cVelCount > 0)) {
    vec2 v_13 = cVel;
    float v_14 = float(cVelCount);
    cVel = (v_13 / vec2(v_14, float(cVelCount)));
  }
  vVel = (((vVel + (cMass * v.inner.rule1Scale)) + (colVel * v.inner.rule2Scale)) + (cVel * v.inner.rule3Scale));
  vec2 v_15 = normalize(vVel);
  vVel = (v_15 * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  vPos = (vPos + (vVel * v.inner.deltaT));
  if ((vPos.x < -1.0f)) {
    vPos[0u] = 1.0f;
  }
  if ((vPos.x > 1.0f)) {
    vPos[0u] = -1.0f;
  }
  if ((vPos.y < -1.0f)) {
    vPos[1u] = 1.0f;
  }
  if ((vPos.y > 1.0f)) {
    vPos[1u] = -1.0f;
  }
  uint v_16 = index;
  v_2.inner.particles[v_16].pos = vPos;
  uint v_17 = index;
  v_2.inner.particles[v_17].vel = vVel;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  comp_main_inner(gl_GlobalInvocationID);
}
