#version 310 es
precision mediump float;


int dimAOuter_1 = 0;
layout (binding = 3) uniform Uniforms_1 {
  float NAN;
  ivec3 aShape;
  ivec3 bShape;
  ivec3 outShape;
  ivec2 outShapeStrides;
} x_48;
int dimInner_1 = 0;
int dimBOuter_1 = 0;
layout (binding = 0) buffer ssbOut_1 {
  float result[];
} x_54;
uvec3 tint_symbol = uvec3(0u, 0u, 0u);
uvec3 tint_symbol_1 = uvec3(0u, 0u, 0u);
shared float mm_Asub[64][64];
shared float mm_Bsub[64][1];
layout (binding = 1) buffer ssbA_1 {
  float A[];
} x_165;
int batch = 0;
layout (binding = 2) buffer ssbB_1 {
  float B[];
} x_185;

bool coordsInBounds_vi2_vi2_(inout ivec2 coord, inout ivec2 shape) {
  bool x_87 = false;
  bool x_88_phi = false;
  ivec2 x_76 = coord;
  bool x_81 = all(greaterThanEqual(x_76, ivec2(0, 0)));
  x_88_phi = x_81;
  if (x_81) {
    ivec2 x_84 = coord;
    ivec2 x_85 = shape;
    x_87 = all(lessThan(x_84, x_85));
    x_88_phi = x_87;
  }
  return x_88_phi;
}

float mm_readA_i1_i1_(inout int row, inout int col) {
  int batchASize = 0;
  ivec2 param_10 = ivec2(0, 0);
  ivec2 param_11 = ivec2(0, 0);
  float x_430 = 0.0f;
  int x_417 = x_48.aShape.y;
  int x_419 = x_48.aShape.z;
  batchASize = (x_417 * x_419);
  int x_421 = row;
  int x_422 = col;
  int x_424 = dimAOuter_1;
  int x_425 = dimInner_1;
  param_10 = ivec2(x_421, x_422);
  param_11 = ivec2(x_424, x_425);
  bool x_429 = coordsInBounds_vi2_vi2_(param_10, param_11);
  if (x_429) {
    int x_438 = batch;
    int x_439 = batchASize;
    int x_441 = row;
    int x_442 = dimInner_1;
    int x_445 = col;
    float x_448 = x_165.A[(((x_438 * x_439) + (x_441 * x_442)) + x_445)];
    x_430 = x_448;
  } else {
    x_430 = 0.0f;
  }
  return x_430;
}

float mm_readB_i1_i1_(inout int row_1, inout int col_1) {
  int batchBSize = 0;
  ivec2 param_12 = ivec2(0, 0);
  ivec2 param_13 = ivec2(0, 0);
  float x_468 = 0.0f;
  int x_455 = x_48.bShape.y;
  int x_457 = x_48.bShape.z;
  batchBSize = (x_455 * x_457);
  int x_459 = row_1;
  int x_460 = col_1;
  int x_462 = dimInner_1;
  int x_463 = dimBOuter_1;
  param_12 = ivec2(x_459, x_460);
  param_13 = ivec2(x_462, x_463);
  bool x_467 = coordsInBounds_vi2_vi2_(param_12, param_13);
  if (x_467) {
    int x_475 = batch;
    int x_476 = batchBSize;
    int x_478 = row_1;
    int x_479 = dimBOuter_1;
    int x_482 = col_1;
    float x_485 = x_185.B[(((x_475 * x_476) + (x_478 * x_479)) + x_482)];
    x_468 = x_485;
  } else {
    x_468 = 0.0f;
  }
  return x_468;
}

int getOutputFlatIndex_vi3_(inout ivec3 coords) {
  ivec3 x_99 = coords;
  int x_105 = x_48.outShapeStrides.x;
  int x_107 = x_48.outShapeStrides.y;
  return int(dot(vec3(x_99), vec3(ivec3(x_105, x_107, 1))));
}

void setOutput_i1_f1_(inout int flatIndex, inout float value) {
  int x_95 = flatIndex;
  float x_96 = value;
  x_54.result[x_95] = x_96;
  return;
}

void setOutput_i1_i1_i1_f1_(inout int d0, inout int d1, inout int d2, inout float value_1) {
  int flatIndex_1 = 0;
  ivec3 param = ivec3(0, 0, 0);
  int param_1 = 0;
  float param_2 = 0.0f;
  int x_115 = d0;
  int x_116 = d1;
  int x_117 = d2;
  param = ivec3(x_115, x_116, x_117);
  int x_120 = getOutputFlatIndex_vi3_(param);
  flatIndex_1 = x_120;
  param_1 = flatIndex_1;
  float x_124 = value_1;
  param_2 = x_124;
  setOutput_i1_f1_(param_1, param_2);
  return;
}

void mm_write_i1_i1_f1_(inout int row_2, inout int col_2, inout float value_2) {
  ivec3 outCoord = ivec3(0, 0, 0);
  int param_14 = 0;
  int param_15 = 0;
  int param_16 = 0;
  float param_17 = 0.0f;
  int x_491 = batch;
  int x_492 = row_2;
  int x_493 = col_2;
  outCoord = ivec3(x_491, x_492, x_493);
  param_14 = batch;
  int x_498 = row_2;
  param_15 = x_498;
  int x_500 = col_2;
  param_16 = x_500;
  float x_502 = value_2;
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
  float acc[1][1] = float[1][1](float[1](0.0f));
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
  float BCached[1] = float[1](0.0f);
  int innerRow_3 = 0;
  float ACached = 0.0f;
  int innerCol_3 = 0;
  int innerRow_4 = 0;
  int innerCol_4 = 0;
  int param_7 = 0;
  int param_8 = 0;
  float param_9 = 0.0f;
  uint x_132 = tint_symbol.y;
  tileRow = (int(x_132) * 1);
  uint x_137 = tint_symbol.x;
  tileCol = (int(x_137) * 1);
  uint x_143 = tint_symbol_1.y;
  globalRow = (int(x_143) * 1);
  uint x_148 = tint_symbol_1.x;
  globalCol = (int(x_148) * 1);
  int x_152 = dimInner;
  numTiles = (((x_152 - 1) / 64) + 1);
  innerRow = 0;
  {
    for(; (innerRow < 1); innerRow = (innerRow + 1)) {
      innerCol = 0;
      {
        for(; (innerCol < 1); innerCol = (innerCol + 1)) {
          acc[innerRow][innerCol] = 0.0f;
        }
      }
    }
  }
  uint x_187 = tint_symbol.x;
  tileColA = (int(x_187) * 64);
  uint x_192 = tint_symbol.y;
  tileRowB = (int(x_192) * 1);
  t = 0;
  {
    for(; (t < numTiles); t = (t + 1)) {
      innerRow_1 = 0;
      {
        for(; (innerRow_1 < 1); innerRow_1 = (innerRow_1 + 1)) {
          innerCol_1 = 0;
          {
            for(; (innerCol_1 < 64); innerCol_1 = (innerCol_1 + 1)) {
              inputRow = (tileRow + innerRow_1);
              inputCol = (tileColA + innerCol_1);
              int x_233 = inputRow;
              int x_234 = inputCol;
              int x_238 = t;
              int x_240 = inputCol;
              param_3 = (globalRow + innerRow_1);
              param_4 = ((x_238 * 64) + x_240);
              float x_244 = mm_readA_i1_i1_(param_3, param_4);
              mm_Asub[x_233][x_234] = x_244;
            }
          }
        }
      }
      innerRow_2 = 0;
      {
        for(; (innerRow_2 < 1); innerRow_2 = (innerRow_2 + 1)) {
          innerCol_2 = 0;
          {
            for(; (innerCol_2 < 1); innerCol_2 = (innerCol_2 + 1)) {
              inputRow_1 = (tileRowB + innerRow_2);
              inputCol_1 = (tileCol + innerCol_2);
              int x_278 = inputRow_1;
              int x_279 = inputCol_1;
              int x_284 = globalCol;
              int x_285 = innerCol_2;
              param_5 = ((t * 64) + inputRow_1);
              param_6 = (x_284 + x_285);
              float x_289 = mm_readB_i1_i1_(param_5, param_6);
              mm_Bsub[x_278][x_279] = x_289;
            }
          }
        }
      }
      memoryBarrierShared();
      k = 0;
      {
        for(; (k < 64); k = (k + 1)) {
          inner = 0;
          {
            for(; (inner < 1); inner = (inner + 1)) {
              int x_314 = inner;
              float x_320 = mm_Bsub[k][(tileCol + inner)];
              BCached[x_314] = x_320;
            }
          }
          innerRow_3 = 0;
          {
            for(; (innerRow_3 < 1); innerRow_3 = (innerRow_3 + 1)) {
              float x_338 = mm_Asub[(tileRow + innerRow_3)][k];
              ACached = x_338;
              innerCol_3 = 0;
              {
                for(; (innerCol_3 < 1); innerCol_3 = (innerCol_3 + 1)) {
                  int x_347 = innerRow_3;
                  int x_348 = innerCol_3;
                  float x_349 = ACached;
                  float x_352 = BCached[innerCol_3];
                  float x_355 = acc[x_347][x_348];
                  acc[x_347][x_348] = (x_355 + (x_349 * x_352));
                }
              }
            }
          }
        }
      }
      memoryBarrierShared();
    }
  }
  innerRow_4 = 0;
  {
    for(; (innerRow_4 < 1); innerRow_4 = (innerRow_4 + 1)) {
      innerCol_4 = 0;
      while (true) {
        bool x_393 = false;
        bool x_394_phi = false;
        if ((innerCol_4 < 1)) {
        } else {
          break;
        }
        int x_382 = globalCol;
        int x_383 = innerCol_4;
        int x_385 = dimBOuter;
        bool x_386 = ((x_382 + x_383) < x_385);
        x_394_phi = x_386;
        if (x_386) {
          int x_389 = globalRow;
          int x_390 = innerRow_4;
          int x_392 = dimAOuter;
          x_393 = ((x_389 + x_390) < x_392);
          x_394_phi = x_393;
        }
        if (x_394_phi) {
          int x_400 = globalCol;
          int x_401 = innerCol_4;
          int x_403 = innerRow_4;
          int x_404 = innerCol_4;
          param_7 = (globalRow + innerRow_4);
          param_8 = (x_400 + x_401);
          float x_409 = acc[x_403][x_404];
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
  int x_67 = x_48.aShape.y;
  dimAOuter_1 = x_67;
  int x_71 = x_48.aShape.z;
  dimInner_1 = x_71;
  int x_75 = x_48.bShape.z;
  dimBOuter_1 = x_75;
  uint x_505 = tint_symbol_1.z;
  batch = int(x_505);
  param_18 = dimAOuter_1;
  param_19 = dimInner_1;
  param_20 = dimBOuter_1;
  mm_matMul_i1_i1_i1_(param_18, param_19, param_20);
  return;
}

struct tint_symbol_6 {
  uvec3 tint_symbol_3;
  uint local_invocation_index;
  uvec3 tint_symbol_4;
};

void tint_symbol_2_inner(uvec3 tint_symbol_3, uvec3 tint_symbol_4, uint local_invocation_index) {
  {
    uint i_1 = local_invocation_index;
    uint i_2 = (local_invocation_index % 1u);
    mm_Bsub[i_1][i_2] = 0.0f;
  }
  {
    for(uint idx = local_invocation_index; (idx < 4096u); idx = (idx + 64u)) {
      uint i = (idx / 64u);
      uint i_1 = (idx % 64u);
      mm_Asub[i][i_1] = 0.0f;
    }
  }
  memoryBarrierShared();
  tint_symbol = tint_symbol_3;
  tint_symbol_1 = tint_symbol_4;
  main_1();
}

layout(local_size_x = 1, local_size_y = 64, local_size_z = 1) in;
void tint_symbol_2(tint_symbol_6 tint_symbol_5) {
  tint_symbol_2_inner(tint_symbol_5.tint_symbol_3, tint_symbol_5.tint_symbol_4, tint_symbol_5.local_invocation_index);
  return;
}
void main() {
  tint_symbol_6 inputs;
  inputs.tint_symbol_3 = gl_LocalInvocationID;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  inputs.tint_symbol_4 = gl_GlobalInvocationID;
  tint_symbol_2(inputs);
}


