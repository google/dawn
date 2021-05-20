struct tint_symbol_1 {
  float2 a_particlePos : TEXCOORD0;
  float2 a_particleVel : TEXCOORD1;
  float2 a_pos : TEXCOORD2;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};
struct tint_symbol_3 {
  float4 value : SV_Target0;
};
struct SimParams {
  float deltaT;
  float rule1Distance;
  float rule2Distance;
  float rule3Distance;
  float rule1Scale;
  float rule2Scale;
  float rule3Scale;
};
struct tint_symbol_5 {
  uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};

ConstantBuffer<SimParams> params : register(b0, space0);

RWByteAddressBuffer particlesA : register(u1, space0);
RWByteAddressBuffer particlesB : register(u2, space0);

tint_symbol_2 vert_main(tint_symbol_1 tint_symbol) {
  const float2 a_particlePos = tint_symbol.a_particlePos;
  const float2 a_particleVel = tint_symbol.a_particleVel;
  const float2 a_pos = tint_symbol.a_pos;
  float angle = -(atan2(a_particleVel.x, a_particleVel.y));
  float2 pos = float2(((a_pos.x * cos(angle)) - (a_pos.y * sin(angle))), ((a_pos.x * sin(angle)) + (a_pos.y * cos(angle))));
  const tint_symbol_2 tint_symbol_8 = {float4((pos + a_particlePos), 0.0f, 1.0f)};
  return tint_symbol_8;
}

tint_symbol_3 frag_main() {
  const tint_symbol_3 tint_symbol_9 = {float4(1.0f, 1.0f, 1.0f, 1.0f)};
  return tint_symbol_9;
}

[numthreads(1, 1, 1)]
void comp_main(tint_symbol_5 tint_symbol_4) {
  const uint3 gl_GlobalInvocationID = tint_symbol_4.gl_GlobalInvocationID;
  uint index = gl_GlobalInvocationID.x;
  if ((index >= 5u)) {
    return;
  }
  float2 vPos = asfloat(particlesA.Load2((16u * index)));
  float2 vVel = asfloat(particlesA.Load2(((16u * index) + 8u)));
  float2 cMass = float2(0.0f, 0.0f);
  float2 cVel = float2(0.0f, 0.0f);
  float2 colVel = float2(0.0f, 0.0f);
  int cMassCount = 0;
  int cVelCount = 0;
  float2 pos = float2(0.0f, 0.0f);
  float2 vel = float2(0.0f, 0.0f);
  {
    uint i = 0u;
    while (true) {
      if (!((i < 5u))) {
        break;
      }
      if ((i == index)) {
        {
          i = (i + 1u);
        }
        continue;
      }
      pos = asfloat(particlesA.Load2((16u * i))).xy;
      vel = asfloat(particlesA.Load2(((16u * i) + 8u))).xy;
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
      {
        i = (i + 1u);
      }
    }
  }
  if ((cMassCount > 0)) {
    cMass = ((cMass / float2(float(cMassCount), float(cMassCount))) - vPos);
  }
  if ((cVelCount > 0)) {
    cVel = (cVel / float2(float(cVelCount), float(cVelCount)));
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
  particlesB.Store2((16u * index), asuint(vPos));
  particlesB.Store2(((16u * index) + 8u), asuint(vVel));
  return;
}

