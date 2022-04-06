static int dimAOuter_1 = 0;
cbuffer cbuffer_x_48 : register(b3, space0) {
  uint4 x_48[5];
};
static int dimInner_1 = 0;
static int dimBOuter_1 = 0;
RWByteAddressBuffer x_54 : register(u0, space0);
static uint3 gl_LocalInvocationID = uint3(0u, 0u, 0u);
static uint3 gl_GlobalInvocationID = uint3(0u, 0u, 0u);
groupshared float mm_Asub[64][64];
groupshared float mm_Bsub[64][1];
ByteAddressBuffer x_165 : register(t1, space0);
static int batch = 0;
ByteAddressBuffer x_185 : register(t2, space0);

bool coordsInBounds_vi2_vi2_(inout int2 coord, inout int2 shape) {
  bool x_87 = false;
  bool x_88_phi = false;
  const int2 x_76 = coord;
  const bool x_81 = all((x_76 >= int2(0, 0)));
  x_88_phi = x_81;
  if (x_81) {
    const int2 x_84 = coord;
    const int2 x_85 = shape;
    x_87 = all((x_84 < x_85));
    x_88_phi = x_87;
  }
  return x_88_phi;
}

float mm_readA_i1_i1_(inout int row, inout int col) {
  int batchASize = 0;
  int2 param_10 = int2(0, 0);
  int2 param_11 = int2(0, 0);
  float x_430 = 0.0f;
  const int x_417 = asint(x_48[1].y);
  const int x_419 = asint(x_48[1].z);
  batchASize = (x_417 * x_419);
  const int x_421 = row;
  const int x_422 = col;
  const int x_424 = dimAOuter_1;
  const int x_425 = dimInner_1;
  param_10 = int2(x_421, x_422);
  param_11 = int2(x_424, x_425);
  const bool x_429 = coordsInBounds_vi2_vi2_(param_10, param_11);
  if (x_429) {
    const int x_438 = batch;
    const int x_439 = batchASize;
    const int x_441 = row;
    const int x_442 = dimInner_1;
    const int x_445 = col;
    const float x_448 = asfloat(x_165.Load((4u * uint((((x_438 * x_439) + (x_441 * x_442)) + x_445)))));
    x_430 = x_448;
  } else {
    x_430 = 0.0f;
  }
  return x_430;
}

float mm_readB_i1_i1_(inout int row_1, inout int col_1) {
  int batchBSize = 0;
  int2 param_12 = int2(0, 0);
  int2 param_13 = int2(0, 0);
  float x_468 = 0.0f;
  const int x_455 = asint(x_48[2].y);
  const int x_457 = asint(x_48[2].z);
  batchBSize = (x_455 * x_457);
  const int x_459 = row_1;
  const int x_460 = col_1;
  const int x_462 = dimInner_1;
  const int x_463 = dimBOuter_1;
  param_12 = int2(x_459, x_460);
  param_13 = int2(x_462, x_463);
  const bool x_467 = coordsInBounds_vi2_vi2_(param_12, param_13);
  if (x_467) {
    const int x_475 = batch;
    const int x_476 = batchBSize;
    const int x_478 = row_1;
    const int x_479 = dimBOuter_1;
    const int x_482 = col_1;
    const float x_485 = asfloat(x_185.Load((4u * uint((((x_475 * x_476) + (x_478 * x_479)) + x_482)))));
    x_468 = x_485;
  } else {
    x_468 = 0.0f;
  }
  return x_468;
}

int getOutputFlatIndex_vi3_(inout int3 coords) {
  const int3 x_99 = coords;
  const int x_105 = asint(x_48[4].x);
  const int x_107 = asint(x_48[4].y);
  return int(dot(float3(x_99), float3(int3(x_105, x_107, 1))));
}

void setOutput_i1_f1_(inout int flatIndex, inout float value) {
  const int x_95 = flatIndex;
  const float x_96 = value;
  x_54.Store((4u * uint(x_95)), asuint(x_96));
  return;
}

void setOutput_i1_i1_i1_f1_(inout int d0, inout int d1, inout int d2, inout float value_1) {
  int flatIndex_1 = 0;
  int3 param = int3(0, 0, 0);
  int param_1 = 0;
  float param_2 = 0.0f;
  const int x_115 = d0;
  const int x_116 = d1;
  const int x_117 = d2;
  param = int3(x_115, x_116, x_117);
  const int x_120 = getOutputFlatIndex_vi3_(param);
  flatIndex_1 = x_120;
  param_1 = flatIndex_1;
  const float x_124 = value_1;
  param_2 = x_124;
  setOutput_i1_f1_(param_1, param_2);
  return;
}

void mm_write_i1_i1_f1_(inout int row_2, inout int col_2, inout float value_2) {
  int3 outCoord = int3(0, 0, 0);
  int param_14 = 0;
  int param_15 = 0;
  int param_16 = 0;
  float param_17 = 0.0f;
  const int x_491 = batch;
  const int x_492 = row_2;
  const int x_493 = col_2;
  outCoord = int3(x_491, x_492, x_493);
  param_14 = batch;
  const int x_498 = row_2;
  param_15 = x_498;
  const int x_500 = col_2;
  param_16 = x_500;
  const float x_502 = value_2;
  param_17 = x_502;
  setOutput_i1_i1_i1_f1_(param_14, param_15, param_16, param_17);
  return;
}

void mm_matMul_i1_i1_i1_(inout int dimAOuter, inout int dimInner, inout int dimBOuter) {
  int tileRow = 0;
  int tileCol = 0;
  int globalRow = 0;
  int globalCol = 0;
  int numTiles = 0;
  int innerRow = 0;
  int innerCol = 0;
  float acc[1][1] = (float[1][1])0;
  int tileColA = 0;
  int tileRowB = 0;
  int t = 0;
  int innerRow_1 = 0;
  int innerCol_1 = 0;
  int inputRow = 0;
  int inputCol = 0;
  int param_3 = 0;
  int param_4 = 0;
  int innerRow_2 = 0;
  int innerCol_2 = 0;
  int inputRow_1 = 0;
  int inputCol_1 = 0;
  int param_5 = 0;
  int param_6 = 0;
  int k = 0;
  int inner = 0;
  float BCached[1] = (float[1])0;
  int innerRow_3 = 0;
  float ACached = 0.0f;
  int innerCol_3 = 0;
  int innerRow_4 = 0;
  int innerCol_4 = 0;
  int param_7 = 0;
  int param_8 = 0;
  float param_9 = 0.0f;
  const uint x_132 = gl_LocalInvocationID.y;
  tileRow = (asint(x_132) * 1);
  const uint x_137 = gl_LocalInvocationID.x;
  tileCol = (asint(x_137) * 1);
  const uint x_143 = gl_GlobalInvocationID.y;
  globalRow = (asint(x_143) * 1);
  const uint x_148 = gl_GlobalInvocationID.x;
  globalCol = (asint(x_148) * 1);
  const int x_152 = dimInner;
  numTiles = (((x_152 - 1) / 64) + 1);
  innerRow = 0;
  {
    [loop] for(; (innerRow < 1); innerRow = (innerRow + 1)) {
      innerCol = 0;
      {
        [loop] for(; (innerCol < 1); innerCol = (innerCol + 1)) {
          acc[innerRow][innerCol] = 0.0f;
        }
      }
    }
  }
  const uint x_187 = gl_LocalInvocationID.x;
  tileColA = (asint(x_187) * 64);
  const uint x_192 = gl_LocalInvocationID.y;
  tileRowB = (asint(x_192) * 1);
  t = 0;
  {
    [loop] for(; (t < numTiles); t = (t + 1)) {
      innerRow_1 = 0;
      {
        [loop] for(; (innerRow_1 < 1); innerRow_1 = (innerRow_1 + 1)) {
          innerCol_1 = 0;
          {
            [loop] for(; (innerCol_1 < 64); innerCol_1 = (innerCol_1 + 1)) {
              inputRow = (tileRow + innerRow_1);
              inputCol = (tileColA + innerCol_1);
              const int x_233 = inputRow;
              const int x_234 = inputCol;
              const int x_238 = t;
              const int x_240 = inputCol;
              param_3 = (globalRow + innerRow_1);
              param_4 = ((x_238 * 64) + x_240);
              const float x_244 = mm_readA_i1_i1_(param_3, param_4);
              mm_Asub[x_233][x_234] = x_244;
            }
          }
        }
      }
      innerRow_2 = 0;
      {
        [loop] for(; (innerRow_2 < 1); innerRow_2 = (innerRow_2 + 1)) {
          innerCol_2 = 0;
          {
            [loop] for(; (innerCol_2 < 1); innerCol_2 = (innerCol_2 + 1)) {
              inputRow_1 = (tileRowB + innerRow_2);
              inputCol_1 = (tileCol + innerCol_2);
              const int x_278 = inputRow_1;
              const int x_279 = inputCol_1;
              const int x_284 = globalCol;
              const int x_285 = innerCol_2;
              param_5 = ((t * 64) + inputRow_1);
              param_6 = (x_284 + x_285);
              const float x_289 = mm_readB_i1_i1_(param_5, param_6);
              mm_Bsub[x_278][x_279] = x_289;
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
      k = 0;
      {
        [loop] for(; (k < 64); k = (k + 1)) {
          inner = 0;
          {
            [loop] for(; (inner < 1); inner = (inner + 1)) {
              const int x_314 = inner;
              const float x_320 = mm_Bsub[k][(tileCol + inner)];
              BCached[x_314] = x_320;
            }
          }
          innerRow_3 = 0;
          {
            [loop] for(; (innerRow_3 < 1); innerRow_3 = (innerRow_3 + 1)) {
              const float x_338 = mm_Asub[(tileRow + innerRow_3)][k];
              ACached = x_338;
              innerCol_3 = 0;
              {
                [loop] for(; (innerCol_3 < 1); innerCol_3 = (innerCol_3 + 1)) {
                  const int x_347 = innerRow_3;
                  const int x_348 = innerCol_3;
                  const float x_349 = ACached;
                  const float x_352 = BCached[innerCol_3];
                  const float x_355 = acc[x_347][x_348];
                  acc[x_347][x_348] = (x_355 + (x_349 * x_352));
                }
              }
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
    }
  }
  innerRow_4 = 0;
  {
    [loop] for(; (innerRow_4 < 1); innerRow_4 = (innerRow_4 + 1)) {
      innerCol_4 = 0;
      [loop] while (true) {
        bool x_393 = false;
        bool x_394_phi = false;
        if ((innerCol_4 < 1)) {
        } else {
          break;
        }
        const int x_382 = globalCol;
        const int x_383 = innerCol_4;
        const int x_385 = dimBOuter;
        const bool x_386 = ((x_382 + x_383) < x_385);
        x_394_phi = x_386;
        if (x_386) {
          const int x_389 = globalRow;
          const int x_390 = innerRow_4;
          const int x_392 = dimAOuter;
          x_393 = ((x_389 + x_390) < x_392);
          x_394_phi = x_393;
        }
        if (x_394_phi) {
          const int x_400 = globalCol;
          const int x_401 = innerCol_4;
          const int x_403 = innerRow_4;
          const int x_404 = innerCol_4;
          param_7 = (globalRow + innerRow_4);
          param_8 = (x_400 + x_401);
          const float x_409 = acc[x_403][x_404];
          param_9 = x_409;
          mm_write_i1_i1_f1_(param_7, param_8, param_9);
        }
        {
          innerCol_4 = (innerCol_4 + 1);
        }
      }
    }
  }
  return;
}

void main_1() {
  int param_18 = 0;
  int param_19 = 0;
  int param_20 = 0;
  const int x_67 = asint(x_48[1].y);
  dimAOuter_1 = x_67;
  const int x_71 = asint(x_48[1].z);
  dimInner_1 = x_71;
  const int x_75 = asint(x_48[2].z);
  dimBOuter_1 = x_75;
  const uint x_505 = gl_GlobalInvocationID.z;
  batch = asint(x_505);
  param_18 = dimAOuter_1;
  param_19 = dimInner_1;
  param_20 = dimBOuter_1;
  mm_matMul_i1_i1_i1_(param_18, param_19, param_20);
  return;
}

struct tint_symbol_1 {
  uint3 gl_LocalInvocationID_param : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 gl_GlobalInvocationID_param : SV_DispatchThreadID;
};

void main_inner(uint3 gl_LocalInvocationID_param, uint3 gl_GlobalInvocationID_param, uint local_invocation_index) {
  {
    const uint i_1 = local_invocation_index;
    const uint i_2 = (local_invocation_index % 1u);
    mm_Bsub[i_1][i_2] = 0.0f;
  }
  {
    [loop] for(uint idx = local_invocation_index; (idx < 4096u); idx = (idx + 64u)) {
      const uint i = (idx / 64u);
      const uint i_1 = (idx % 64u);
      mm_Asub[i][i_1] = 0.0f;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  gl_LocalInvocationID = gl_LocalInvocationID_param;
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

[numthreads(1, 64, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gl_LocalInvocationID_param, tint_symbol.gl_GlobalInvocationID_param, tint_symbol.local_invocation_index);
  return;
}
