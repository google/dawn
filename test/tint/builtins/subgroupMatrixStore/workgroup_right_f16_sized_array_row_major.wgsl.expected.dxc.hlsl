#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::B, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint out0[128];
groupshared uint2 out1[128];
groupshared uint out2[512];
groupshared uint4 out3[128];
groupshared float16_t out4[128];
groupshared vector<float16_t, 2> out5[128];
groupshared vector<float16_t, 3> out6[128];
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
      uint v_1 = ((v * 16u) / 4u);
      out2[v_1] = 0u;
      uint v_2 = (v_1 + 1u);
      out2[v_2] = 0u;
      out2[(v_2 + 1u)] = 0u;
      out3[((v * 16u) / 16u)] = (0u).xxxx;
      out4[v] = float16_t(0.0h);
      out5[v] = (float16_t(0.0h)).xx;
      out6[v] = (float16_t(0.0h)).xxx;
      {
        idx = (idx + 64u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  Matrix_right_f16_8x8 m = Matrix_right_f16_8x8::Splat(float16_t(0.0h));
  m.Store(out0, 0u, 32u, MatrixLayout::RowMajor);
  m.Store(out1, 0u, 64u, MatrixLayout::RowMajor);
  m.Store(out2, 0u, 128u, MatrixLayout::RowMajor);
  m.Store(out3, 0u, 128u, MatrixLayout::RowMajor);
  m.Store(out4, 0u, 16u, MatrixLayout::RowMajor);
  m.Store(out5, 0u, 32u, MatrixLayout::RowMajor);
  m.Store(out6, 0u, 64u, MatrixLayout::RowMajor);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

