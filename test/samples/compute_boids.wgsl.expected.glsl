#version 310 es
precision mediump float;

struct tint_symbol_2 {
  vec2 a_particlePos;
  vec2 a_particleVel;
  vec2 a_pos;
};
struct tint_symbol_3 {
  vec4 value;
};

vec4 vert_main_inner(vec2 a_particlePos, vec2 a_particleVel, vec2 a_pos) {
  float angle = -(atan(a_particleVel.x, a_particleVel.y));
  vec2 pos = vec2(((a_pos.x * cos(angle)) - (a_pos.y * sin(angle))), ((a_pos.x * sin(angle)) + (a_pos.y * cos(angle))));
  return vec4((pos + a_particlePos), 0.0f, 1.0f);
}

struct tint_symbol_4 {
  vec4 value;
};
struct Particle {
  vec2 pos;
  vec2 vel;
};
struct tint_symbol_6 {
  uvec3 tint_symbol;
};

tint_symbol_3 vert_main(tint_symbol_2 tint_symbol_1) {
  vec4 inner_result = vert_main_inner(tint_symbol_1.a_particlePos, tint_symbol_1.a_particleVel, tint_symbol_1.a_pos);
  tint_symbol_3 wrapper_result = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
in vec2 a_particlePos;
in vec2 a_particleVel;
in vec2 a_pos;
void main() {
  tint_symbol_2 inputs;
  inputs.a_particlePos = a_particlePos;
  inputs.a_particleVel = a_particleVel;
  inputs.a_pos = a_pos;
  tint_symbol_3 outputs;
  outputs = vert_main(inputs);
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

struct tint_symbol_2 {
  vec2 a_particlePos;
  vec2 a_particleVel;
  vec2 a_pos;
};
struct tint_symbol_3 {
  vec4 value;
};
struct tint_symbol_4 {
  vec4 value;
};

vec4 frag_main_inner() {
  return vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

struct Particle {
  vec2 pos;
  vec2 vel;
};
struct tint_symbol_6 {
  uvec3 tint_symbol;
};

tint_symbol_4 frag_main() {
  vec4 inner_result_1 = frag_main_inner();
  tint_symbol_4 wrapper_result_1 = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
out vec4 value;
void main() {
  tint_symbol_4 outputs;
  outputs = frag_main();
  value = outputs.value;
}


#version 310 es
precision mediump float;

struct tint_symbol_2 {
  vec2 a_particlePos;
  vec2 a_particleVel;
  vec2 a_pos;
};
struct tint_symbol_3 {
  vec4 value;
};
struct tint_symbol_4 {
  vec4 value;
};
struct Particle {
  vec2 pos;
  vec2 vel;
};

layout (binding = 0) uniform SimParams_1 {
  float deltaT;
  float rule1Distance;
  float rule2Distance;
  float rule3Distance;
  float rule1Scale;
  float rule2Scale;
  float rule3Scale;
} params;
layout (binding = 1) buffer Particles_1 {
  Particle particles[5];
} particlesA;
layout (binding = 2) buffer Particles_2 {
  Particle particles[5];
} particlesB;

struct tint_symbol_6 {
  uvec3 tint_symbol;
};

void comp_main_inner(uvec3 tint_symbol) {
  uint index = tint_symbol.x;
  if ((index >= 5u)) {
    return;
  }
  vec2 vPos = particlesA.particles[index].pos;
  vec2 vVel = particlesA.particles[index].vel;
  vec2 cMass = vec2(0.0f, 0.0f);
  vec2 cVel = vec2(0.0f, 0.0f);
  vec2 colVel = vec2(0.0f, 0.0f);
  int cMassCount = 0;
  int cVelCount = 0;
  vec2 pos = vec2(0.0f, 0.0f);
  vec2 vel = vec2(0.0f, 0.0f);
  {
    for(uint i = 0u; (i < 5u); i = (i + 1u)) {
      if ((i == index)) {
        continue;
      }
      pos = particlesA.particles[i].pos.xy;
      vel = particlesA.particles[i].vel.xy;
      if ((distance(pos, vPos) < params.rule1Distance)) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      if ((distance(pos, vPos) < params.rule2Distance)) {
        colVel = (colVel - (pos - vPos));
      }
      if ((distance(pos, vPos) < params.rule3Distance)) {
        cVel = (cVel + vel);
        cVelCount = (cVelCount + 1);
      }
    }
  }
  if ((cMassCount > 0)) {
    cMass = ((cMass / vec2(float(cMassCount), float(cMassCount))) - vPos);
  }
  if ((cVelCount > 0)) {
    cVel = (cVel / vec2(float(cVelCount), float(cVelCount)));
  }
  vVel = (((vVel + (cMass * params.rule1Scale)) + (colVel * params.rule2Scale)) + (cVel * params.rule3Scale));
  vVel = (normalize(vVel) * clamp(length(vVel), 0.0f, 0.100000001f));
  vPos = (vPos + (vVel * params.deltaT));
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
  particlesB.particles[index].pos = vPos;
  particlesB.particles[index].vel = vVel;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void comp_main(tint_symbol_6 tint_symbol_5) {
  comp_main_inner(tint_symbol_5.tint_symbol);
  return;
}
void main() {
  tint_symbol_6 inputs;
  inputs.tint_symbol = gl_GlobalInvocationID;
  comp_main(inputs);
}


