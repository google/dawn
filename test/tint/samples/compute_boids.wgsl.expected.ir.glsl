SKIP: FAILED

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
precision highp float;
precision highp int;


uniform SimParams params;
Particles particlesA;
Particles particlesB;
vec4 main(vec2 a_particlePos, vec2 a_particleVel, vec2 a_pos) {
  float angle = -(atan(a_particleVel[0u], a_particleVel[1u]));
  float v = (a_pos[0u] * cos(angle));
  float v_1 = (v - (a_pos[1u] * sin(angle)));
  float v_2 = (a_pos[0u] * sin(angle));
  vec2 pos = vec2(v_1, (v_2 + (a_pos[1u] * cos(angle))));
  return vec4((pos + a_particlePos), 0.0f, 1.0f);
}
vec4 main() {
  return vec4(1.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(uvec3 tint_symbol) {
  uint index = tint_symbol[0u];
  if ((index >= 5u)) {
    return;
  }
  vec2 vPos = particlesA.particles[index].pos;
  vec2 vVel = particlesA.particles[index].vel;
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
      pos = particlesA.particles[i].pos.xy;
      vel = particlesA.particles[i].vel.xy;
      float v_3 = distance(pos, vPos);
      if ((v_3 < params.rule1Distance)) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      float v_4 = distance(pos, vPos);
      if ((v_4 < params.rule2Distance)) {
        colVel = (colVel - (pos - vPos));
      }
      float v_5 = distance(pos, vPos);
      if ((v_5 < params.rule3Distance)) {
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
    vec2 v_6 = cMass;
    float v_7 = float(cMassCount);
    vec2 v_8 = (v_6 / vec2(v_7, float(cMassCount)));
    cMass = (v_8 - vPos);
  }
  if ((cVelCount > 0)) {
    vec2 v_9 = cVel;
    float v_10 = float(cVelCount);
    cVel = (v_9 / vec2(v_10, float(cVelCount)));
  }
  vVel = (((vVel + (cMass * params.rule1Scale)) + (colVel * params.rule2Scale)) + (cVel * params.rule3Scale));
  vec2 v_11 = normalize(vVel);
  vVel = (v_11 * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  vPos = (vPos + (vVel * params.deltaT));
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
  particlesB.particles[index].pos = vPos;
  particlesB.particles[index].vel = vVel;
}
error: Error parsing GLSL shader:
ERROR: 0:28: 'main' : function cannot take any parameter(s) 
ERROR: 0:28: 'float' :  entry point cannot return a value
ERROR: 0:28: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



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
precision highp float;
precision highp int;


uniform SimParams params;
Particles particlesA;
Particles particlesB;
vec4 main(vec2 a_particlePos, vec2 a_particleVel, vec2 a_pos) {
  float angle = -(atan(a_particleVel[0u], a_particleVel[1u]));
  float v = (a_pos[0u] * cos(angle));
  float v_1 = (v - (a_pos[1u] * sin(angle)));
  float v_2 = (a_pos[0u] * sin(angle));
  vec2 pos = vec2(v_1, (v_2 + (a_pos[1u] * cos(angle))));
  return vec4((pos + a_particlePos), 0.0f, 1.0f);
}
vec4 main() {
  return vec4(1.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(uvec3 tint_symbol) {
  uint index = tint_symbol[0u];
  if ((index >= 5u)) {
    return;
  }
  vec2 vPos = particlesA.particles[index].pos;
  vec2 vVel = particlesA.particles[index].vel;
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
      pos = particlesA.particles[i].pos.xy;
      vel = particlesA.particles[i].vel.xy;
      float v_3 = distance(pos, vPos);
      if ((v_3 < params.rule1Distance)) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      float v_4 = distance(pos, vPos);
      if ((v_4 < params.rule2Distance)) {
        colVel = (colVel - (pos - vPos));
      }
      float v_5 = distance(pos, vPos);
      if ((v_5 < params.rule3Distance)) {
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
    vec2 v_6 = cMass;
    float v_7 = float(cMassCount);
    vec2 v_8 = (v_6 / vec2(v_7, float(cMassCount)));
    cMass = (v_8 - vPos);
  }
  if ((cVelCount > 0)) {
    vec2 v_9 = cVel;
    float v_10 = float(cVelCount);
    cVel = (v_9 / vec2(v_10, float(cVelCount)));
  }
  vVel = (((vVel + (cMass * params.rule1Scale)) + (colVel * params.rule2Scale)) + (cVel * params.rule3Scale));
  vec2 v_11 = normalize(vVel);
  vVel = (v_11 * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  vPos = (vPos + (vVel * params.deltaT));
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
  particlesB.particles[index].pos = vPos;
  particlesB.particles[index].vel = vVel;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



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
precision highp float;
precision highp int;


uniform SimParams params;
Particles particlesA;
Particles particlesB;
vec4 main(vec2 a_particlePos, vec2 a_particleVel, vec2 a_pos) {
  float angle = -(atan(a_particleVel[0u], a_particleVel[1u]));
  float v = (a_pos[0u] * cos(angle));
  float v_1 = (v - (a_pos[1u] * sin(angle)));
  float v_2 = (a_pos[0u] * sin(angle));
  vec2 pos = vec2(v_1, (v_2 + (a_pos[1u] * cos(angle))));
  return vec4((pos + a_particlePos), 0.0f, 1.0f);
}
vec4 main() {
  return vec4(1.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(uvec3 tint_symbol) {
  uint index = tint_symbol[0u];
  if ((index >= 5u)) {
    return;
  }
  vec2 vPos = particlesA.particles[index].pos;
  vec2 vVel = particlesA.particles[index].vel;
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
      pos = particlesA.particles[i].pos.xy;
      vel = particlesA.particles[i].vel.xy;
      float v_3 = distance(pos, vPos);
      if ((v_3 < params.rule1Distance)) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      float v_4 = distance(pos, vPos);
      if ((v_4 < params.rule2Distance)) {
        colVel = (colVel - (pos - vPos));
      }
      float v_5 = distance(pos, vPos);
      if ((v_5 < params.rule3Distance)) {
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
    vec2 v_6 = cMass;
    float v_7 = float(cMassCount);
    vec2 v_8 = (v_6 / vec2(v_7, float(cMassCount)));
    cMass = (v_8 - vPos);
  }
  if ((cVelCount > 0)) {
    vec2 v_9 = cVel;
    float v_10 = float(cVelCount);
    cVel = (v_9 / vec2(v_10, float(cVelCount)));
  }
  vVel = (((vVel + (cMass * params.rule1Scale)) + (colVel * params.rule2Scale)) + (cVel * params.rule3Scale));
  vec2 v_11 = normalize(vVel);
  vVel = (v_11 * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  vPos = (vPos + (vVel * params.deltaT));
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
  particlesB.particles[index].pos = vPos;
  particlesB.particles[index].vel = vVel;
}
error: Error parsing GLSL shader:
ERROR: 0:28: 'main' : function cannot take any parameter(s) 
ERROR: 0:28: 'float' :  entry point cannot return a value
ERROR: 0:28: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
