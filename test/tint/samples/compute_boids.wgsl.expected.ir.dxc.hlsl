struct vert_main_outputs {
  float4 tint_symbol : SV_Position;
};

struct vert_main_inputs {
  float2 a_particlePos : TEXCOORD0;
  float2 a_particleVel : TEXCOORD1;
  float2 a_pos : TEXCOORD2;
};

struct frag_main_outputs {
  float4 tint_symbol_1 : SV_Target0;
};

struct comp_main_inputs {
  uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_params : register(b0) {
  uint4 params[2];
};
RWByteAddressBuffer particlesA : register(u1);
RWByteAddressBuffer particlesB : register(u2);
float4 vert_main_inner(float2 a_particlePos, float2 a_particleVel, float2 a_pos) {
  float angle = -(atan2(a_particleVel[0u], a_particleVel[1u]));
  float v = (a_pos[0u] * cos(angle));
  float v_1 = (v - (a_pos[1u] * sin(angle)));
  float v_2 = (a_pos[0u] * sin(angle));
  float2 pos = float2(v_1, (v_2 + (a_pos[1u] * cos(angle))));
  return float4((pos + a_particlePos), 0.0f, 1.0f);
}

float4 frag_main_inner() {
  return (1.0f).xxxx;
}

void comp_main_inner(uint3 gl_GlobalInvocationID) {
  uint index = gl_GlobalInvocationID[0u];
  if ((index >= 5u)) {
    return;
  }
  float2 vPos = asfloat(particlesA.Load2((0u + (uint(index) * 16u))));
  float2 vVel = asfloat(particlesA.Load2((8u + (uint(index) * 16u))));
  float2 cMass = (0.0f).xx;
  float2 cVel = (0.0f).xx;
  float2 colVel = (0.0f).xx;
  int cMassCount = int(0);
  int cVelCount = int(0);
  float2 pos = (0.0f).xx;
  float2 vel = (0.0f).xx;
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
      pos = asfloat(particlesA.Load2((0u + (uint(i) * 16u)))).xy;
      vel = asfloat(particlesA.Load2((8u + (uint(i) * 16u)))).xy;
      float v_3 = distance(pos, vPos);
      if ((v_3 < asfloat(params[0u].y))) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + int(1));
      }
      float v_4 = distance(pos, vPos);
      if ((v_4 < asfloat(params[0u].z))) {
        colVel = (colVel - (pos - vPos));
      }
      float v_5 = distance(pos, vPos);
      if ((v_5 < asfloat(params[0u].w))) {
        cVel = (cVel + vel);
        cVelCount = (cVelCount + int(1));
      }
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  if ((cMassCount > int(0))) {
    float2 v_6 = cMass;
    float v_7 = float(cMassCount);
    float2 v_8 = (v_6 / float2(v_7, float(cMassCount)));
    cMass = (v_8 - vPos);
  }
  if ((cVelCount > int(0))) {
    float2 v_9 = cVel;
    float v_10 = float(cVelCount);
    cVel = (v_9 / float2(v_10, float(cVelCount)));
  }
  float2 v_11 = vVel;
  float2 v_12 = cMass;
  float2 v_13 = (v_11 + (v_12 * asfloat(params[1u].x)));
  float2 v_14 = colVel;
  float2 v_15 = (v_13 + (v_14 * asfloat(params[1u].y)));
  float2 v_16 = cVel;
  vVel = (v_15 + (v_16 * asfloat(params[1u].z)));
  float2 v_17 = normalize(vVel);
  vVel = (v_17 * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  float2 v_18 = vPos;
  float2 v_19 = vVel;
  vPos = (v_18 + (v_19 * asfloat(params[0u].x)));
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
  uint v_20 = (uint(index) * 16u);
  particlesB.Store2((0u + v_20), asuint(vPos));
  uint v_21 = (uint(index) * 16u);
  particlesB.Store2((8u + v_21), asuint(vVel));
}

vert_main_outputs vert_main(vert_main_inputs inputs) {
  vert_main_outputs v_22 = {vert_main_inner(inputs.a_particlePos, inputs.a_particleVel, inputs.a_pos)};
  return v_22;
}

frag_main_outputs frag_main() {
  frag_main_outputs v_23 = {frag_main_inner()};
  return v_23;
}

[numthreads(1, 1, 1)]
void comp_main(comp_main_inputs inputs) {
  comp_main_inner(inputs.gl_GlobalInvocationID);
}

