#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
using Matrix_right_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::B, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer s_var : register(u0);
groupshared float wg_var[1024];
void main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 1024u)) {
        break;
      }
      wg_var[v_1] = 0.0f;
      {
        v = (v_1 + 32u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_2 = 0u;
  s_var.GetDimensions(v_2);
  uint v_3 = asuint(int(0));
  uint v_4 = asuint(int(8));
  bool v_5 = (((v_3 + (v_4 * 7u)) + 8u) <= (v_2 / 4u));
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Load(s_var, (0u + (select(v_5, v_3, 0u) * 4u)), (select(v_5, v_4, 8u) * 4u), MatrixLayout::RowMajor);
  m.Store(wg_var, asuint(int(0)), asuint(int(8)), MatrixLayout::RowMajor);
  GroupMemoryBarrierWithGroupSync();
  Matrix_right_f32_8x8 m2 = Matrix_right_f32_8x8::Load(wg_var, asuint(int(0)), asuint(int(8)), MatrixLayout::ColMajor);
  uint v_6 = 0u;
  s_var.GetDimensions(v_6);
  uint v_7 = asuint(int(0));
  uint v_8 = asuint(int(8));
  bool v_9 = (((v_7 + (v_8 * 7u)) + 8u) <= (v_6 / 4u));
  m2.Store(s_var, (0u + (select(v_9, v_7, 0u) * 4u)), (select(v_9, v_8, 8u) * 4u), MatrixLayout::ColMajor);
}

[numthreads(32, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

