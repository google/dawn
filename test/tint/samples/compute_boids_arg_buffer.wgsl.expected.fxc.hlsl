struct comp_main_inputs {
  uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_params : register(b0) {
  uint4 params[2];
};
RWByteAddressBuffer particlesA : register(u0, space1);
RWByteAddressBuffer particlesB : register(u0, space2);
void comp_main_inner(uint3 gl_GlobalInvocationID) {
  uint index = gl_GlobalInvocationID.x;
  if ((index >= 5u)) {
    return;
  }
  float2 vPos = asfloat(particlesA.Load2((0u + (min(index, 4u) * 16u))));
  float2 vVel = asfloat(particlesA.Load2((8u + (min(index, 4u) * 16u))));
  float2 cMass = (0.0f).xx;
  float2 cVel = (0.0f).xx;
  float2 colVel = (0.0f).xx;
  int cMassCount = int(0);
  int cVelCount = int(0);
  float2 pos = (0.0f).xx;
  float2 vel = (0.0f).xx;
  {
    uint i = 0u;
    while((i < 5u)) {
      if ((i == index)) {
        {
          i = (i + 1u);
        }
        continue;
      }
      pos = asfloat(particlesA.Load2((0u + (i * 16u)))).xy;
      vel = asfloat(particlesA.Load2((8u + (i * 16u)))).xy;
      if ((distance(pos, vPos) < asfloat(params[0u].y))) {
        cMass = (cMass + pos);
        cMassCount = asint((asuint(cMassCount) + asuint(int(1))));
      }
      if ((distance(pos, vPos) < asfloat(params[0u].z))) {
        colVel = (colVel - (pos - vPos));
      }
      if ((distance(pos, vPos) < asfloat(params[0u].w))) {
        cVel = (cVel + vel);
        cVelCount = asint((asuint(cVelCount) + asuint(int(1))));
      }
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  if ((cMassCount > int(0))) {
    float2 v = cMass;
    float v_1 = float(cMassCount);
    float2 v_2 = (v / float2(v_1, float(cMassCount)));
    cMass = (v_2 - vPos);
  }
  if ((cVelCount > int(0))) {
    float2 v_3 = cVel;
    float v_4 = float(cVelCount);
    cVel = (v_3 / float2(v_4, float(cVelCount)));
  }
  vVel = (((vVel + (cMass * asfloat(params[1u].x))) + (colVel * asfloat(params[1u].y))) + (cVel * asfloat(params[1u].z)));
  vVel = (normalize(vVel) * clamp(length(vVel), 0.0f, 0.10000000149011611938f));
  vPos = (vPos + (vVel * asfloat(params[0u].x)));
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
  particlesB.Store2((0u + (min(index, 4u) * 16u)), asuint(vPos));
  particlesB.Store2((8u + (min(index, 4u) * 16u)), asuint(vVel));
}

[numthreads(1, 1, 1)]
void comp_main(comp_main_inputs inputs) {
  comp_main_inner(inputs.gl_GlobalInvocationID);
}

