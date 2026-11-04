#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::B, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint out0[128];
groupshared uint2 out1[128];
groupshared float3 out2[128];
groupshared uint4 out3[128];
groupshared uint16_t out5[256];
groupshared uint16_t out6[512];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 128u)) {
        break;
      }
      out0[((v * 4u) / 4u)] = 0u;
      out1[((v * 8u) / 8u)] = (0u).xx;
      out2[v] = (0.0f).xxx;
      out3[((v * 16u) / 16u)] = (0u).xxxx;
      uint v_1 = ((v * 4u) / 2u);
      out5[v_1] = uint16_t(0u);
      out5[(v_1 + 1u)] = uint16_t(0u);
      uint v_2 = ((v * 8u) / 2u);
      out6[v_2] = uint16_t(0u);
      uint v_3 = (v_2 + 1u);
      out6[v_3] = uint16_t(0u);
      out6[(v_3 + 1u)] = uint16_t(0u);
      {
        idx = (idx + 64u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  Matrix_right_f32_8x8 m = Matrix_right_f32_8x8::Splat(0.0f);
  m.Store(out0, 0u, 16u, MatrixLayout::RowMajor);
  m.Store(out1, 0u, 32u, MatrixLayout::RowMajor);
  m.Store(out2, 0u, 64u, MatrixLayout::RowMajor);
  m.Store(out3, 0u, 64u, MatrixLayout::RowMajor);
  m.Store(out5, 0u, 16u, MatrixLayout::RowMajor);
  m.Store(out6, 0u, 32u, MatrixLayout::RowMajor);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

