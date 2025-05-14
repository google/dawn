cbuffer cbuffer_params : register(b0) {
  uint4 params[2];
};
RWByteAddressBuffer particlesA : register(u1);
RWByteAddressBuffer particlesB : register(u2);

struct tint_symbol_1 {
  uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};

void comp_main_inner(uint3 gl_GlobalInvocationID) {
  uint index = gl_GlobalInvocationID.x;
  if ((index >= 5u)) {
    return;
  }
  float2 vPos = asfloat(particlesA.Load2((16u * min(index, 4u))));
  float2 vVel = asfloat(particlesA.Load2(((16u * min(index, 4u)) + 8u)));
  float2 cMass = (0.0f).xx;
  float2 cVel = (0.0f).xx;
  float2 colVel = (0.0f).xx;
  int cMassCount = 0;
  int cVelCount = 0;
  float2 pos = float2(0.0f, 0.0f);
  float2 vel = float2(0.0f, 0.0f);
  {
    for(uint i = 0u; (i < 5u); i = (i + 1u)) {
      if ((i == index)) {
        continue;
      }
      pos = asfloat(particlesA.Load2((16u * min(i, 4u)))).xy;
      vel = asfloat(particlesA.Load2(((16u * min(i, 4u)) + 8u))).xy;
      if ((distance(pos, vPos) < asfloat(params[0].y))) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      if ((distance(pos, vPos) < asfloat(params[0].z))) {
        colVel = (colVel - (pos - vPos));
      }
      if ((distance(pos, vPos) < asfloat(params[0].w))) {
        cVel = (cVel + vel);
        cVelCount = (cVelCount + 1);
      }
    }
  }
  if ((cMassCount > 0)) {
    cMass = ((cMass / float2(float(cMassCount), float(cMassCount))) - vPos);
  }
  if ((cVelCount > 0)) {
    cVel = (cVel / float2(float(cVelCount), float(cVelCount)));
  }
  vVel = (((vVel + (cMass * asfloat(params[1].x))) + (colVel * asfloat(params[1].y))) + (cVel * asfloat(params[1].z)));
  vVel = (normalize(vVel) * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  vPos = (vPos + (vVel * asfloat(params[0].x)));
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
  particlesB.Store2((16u * min(index, 4u)), asuint(vPos));
  particlesB.Store2(((16u * min(index, 4u)) + 8u), asuint(vVel));
}

[numthreads(1, 1, 1)]
void comp_main(tint_symbol_1 tint_symbol) {
  comp_main_inner(tint_symbol.gl_GlobalInvocationID);
  return;
}
