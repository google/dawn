#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint in0[128];
groupshared uint2 in1[128];
groupshared uint in2[512];
groupshared uint4 in3[128];
groupshared float16_t in4[128];
groupshared vector<float16_t, 2> in5[128];
groupshared vector<float16_t, 3> in6[128];
RWByteAddressBuffer v : register(u0);
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 128u)) {
        break;
      }
      in0[((v_1 * 4u) / 4u)] = 0u;
      in1[((v_1 * 8u) / 8u)] = (0u).xx;
      uint v_2 = ((v_1 * 16u) / 4u);
      in2[v_2] = 0u;
      uint v_3 = (v_2 + 1u);
      in2[v_3] = 0u;
      in2[(v_3 + 1u)] = 0u;
      in3[((v_1 * 16u) / 16u)] = (0u).xxxx;
      in4[v_1] = float16_t(0.0h);
      in5[v_1] = (float16_t(0.0h)).xx;
      in6[v_1] = (float16_t(0.0h)).xxx;
      {
        idx = (idx + 64u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  Matrix_result_f16_8x8 m0 = Matrix_result_f16_8x8::Load(in0, 0u, 32u, MatrixLayout::ColMajor);
  uint v_4 = 0u;
  v.GetDimensions(v_4);
  bool v_5 = (116u <= (v_4 / 4u));
  m0.Store(v, (0u + (select(v_5, 0u, 0u) * 4u)), (select(v_5, 16u, 4u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f16_8x8 m1 = Matrix_result_f16_8x8::Load(in1, 0u, 64u, MatrixLayout::ColMajor);
  uint v_6 = 0u;
  v.GetDimensions(v_6);
  bool v_7 = (116u <= (v_6 / 4u));
  m1.Store(v, (0u + (select(v_7, 0u, 0u) * 4u)), (select(v_7, 16u, 4u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f16_8x8 m2 = Matrix_result_f16_8x8::Load(in2, 0u, 128u, MatrixLayout::ColMajor);
  uint v_8 = 0u;
  v.GetDimensions(v_8);
  bool v_9 = (116u <= (v_8 / 4u));
  m2.Store(v, (0u + (select(v_9, 0u, 0u) * 4u)), (select(v_9, 16u, 4u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f16_8x8 m3 = Matrix_result_f16_8x8::Load(in3, 0u, 128u, MatrixLayout::ColMajor);
  uint v_10 = 0u;
  v.GetDimensions(v_10);
  bool v_11 = (116u <= (v_10 / 4u));
  m3.Store(v, (0u + (select(v_11, 0u, 0u) * 4u)), (select(v_11, 16u, 4u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f16_8x8 m4 = Matrix_result_f16_8x8::Load(in4, 0u, 16u, MatrixLayout::ColMajor);
  uint v_12 = 0u;
  v.GetDimensions(v_12);
  bool v_13 = (116u <= (v_12 / 4u));
  m4.Store(v, (0u + (select(v_13, 0u, 0u) * 4u)), (select(v_13, 16u, 4u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f16_8x8 m5 = Matrix_result_f16_8x8::Load(in5, 0u, 32u, MatrixLayout::ColMajor);
  uint v_14 = 0u;
  v.GetDimensions(v_14);
  bool v_15 = (116u <= (v_14 / 4u));
  m5.Store(v, (0u + (select(v_15, 0u, 0u) * 4u)), (select(v_15, 16u, 4u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_f16_8x8 m6 = Matrix_result_f16_8x8::Load(in6, 0u, 64u, MatrixLayout::ColMajor);
  uint v_16 = 0u;
  v.GetDimensions(v_16);
  bool v_17 = (116u <= (v_16 / 4u));
  m6.Store(v, (0u + (select(v_17, 0u, 0u) * 4u)), (select(v_17, 16u, 4u) * 4u), MatrixLayout::ColMajor);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

