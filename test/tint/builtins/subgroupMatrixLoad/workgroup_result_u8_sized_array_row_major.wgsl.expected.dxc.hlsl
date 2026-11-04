#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u8_8x8 = Matrix<ComponentType::U8, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint in0[128];
groupshared uint2 in1[128];
groupshared uint in2[512];
groupshared uint4 in3[128];
groupshared uint16_t in4[128];
groupshared uint16_t in5[256];
groupshared uint16_t in6[512];
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
      in4[((v_1 * 2u) / 2u)] = uint16_t(0u);
      uint v_4 = ((v_1 * 4u) / 2u);
      in5[v_4] = uint16_t(0u);
      in5[(v_4 + 1u)] = uint16_t(0u);
      uint v_5 = ((v_1 * 8u) / 2u);
      in6[v_5] = uint16_t(0u);
      uint v_6 = (v_5 + 1u);
      in6[v_6] = uint16_t(0u);
      in6[(v_6 + 1u)] = uint16_t(0u);
      {
        idx = (idx + 64u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  Matrix_result_u8_8x8 m0 = Matrix_result_u8_8x8::Load(in0, 0u, 64u, MatrixLayout::RowMajor);
  uint v_7 = 0u;
  v.GetDimensions(v_7);
  bool v_8 = (114u <= (v_7 / 4u));
  m0.Store(v, (0u + (select(v_8, 0u, 0u) * 4u)), (select(v_8, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_u8_8x8 m1 = Matrix_result_u8_8x8::Load(in1, 0u, 128u, MatrixLayout::RowMajor);
  uint v_9 = 0u;
  v.GetDimensions(v_9);
  bool v_10 = (114u <= (v_9 / 4u));
  m1.Store(v, (0u + (select(v_10, 0u, 0u) * 4u)), (select(v_10, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_u8_8x8 m2 = Matrix_result_u8_8x8::Load(in2, 0u, 256u, MatrixLayout::RowMajor);
  uint v_11 = 0u;
  v.GetDimensions(v_11);
  bool v_12 = (114u <= (v_11 / 4u));
  m2.Store(v, (0u + (select(v_12, 0u, 0u) * 4u)), (select(v_12, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_u8_8x8 m3 = Matrix_result_u8_8x8::Load(in3, 0u, 256u, MatrixLayout::RowMajor);
  uint v_13 = 0u;
  v.GetDimensions(v_13);
  bool v_14 = (114u <= (v_13 / 4u));
  m3.Store(v, (0u + (select(v_14, 0u, 0u) * 4u)), (select(v_14, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_u8_8x8 m4 = Matrix_result_u8_8x8::Load(in4, 0u, 32u, MatrixLayout::RowMajor);
  uint v_15 = 0u;
  v.GetDimensions(v_15);
  bool v_16 = (114u <= (v_15 / 4u));
  m4.Store(v, (0u + (select(v_16, 0u, 0u) * 4u)), (select(v_16, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_u8_8x8 m5 = Matrix_result_u8_8x8::Load(in5, 0u, 64u, MatrixLayout::RowMajor);
  uint v_17 = 0u;
  v.GetDimensions(v_17);
  bool v_18 = (114u <= (v_17 / 4u));
  m5.Store(v, (0u + (select(v_18, 0u, 0u) * 4u)), (select(v_18, 16u, 2u) * 4u), MatrixLayout::ColMajor);
  Matrix_result_u8_8x8 m6 = Matrix_result_u8_8x8::Load(in6, 0u, 128u, MatrixLayout::RowMajor);
  uint v_19 = 0u;
  v.GetDimensions(v_19);
  bool v_20 = (114u <= (v_19 / 4u));
  m6.Store(v, (0u + (select(v_20, 0u, 0u) * 4u)), (select(v_20, 16u, 2u) * 4u), MatrixLayout::ColMajor);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

