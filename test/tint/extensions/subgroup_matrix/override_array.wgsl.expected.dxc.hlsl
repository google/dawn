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
  uint v_5 = (v_3 + select((((v_4 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_4))), 4294967295u, (v_4 * 7u)));
  uint v_6 = select((v_5 < v_3), 4294967295u, v_5);
  uint v_7 = (v_6 + 8u);
  bool v_8 = (select((v_7 < v_6), 4294967295u, v_7) <= (v_2 / 4u));
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Load(s_var, (0u + (select(v_8, v_3, 0u) * 4u)), (select(v_8, v_4, 8u) * 4u), MatrixLayout::RowMajor);
  m.Store(wg_var, 0u, 8u, MatrixLayout::RowMajor);
  GroupMemoryBarrierWithGroupSync();
  Matrix_right_f32_8x8 m2 = Matrix_right_f32_8x8::Load(wg_var, 0u, 8u, MatrixLayout::ColMajor);
  uint v_9 = 0u;
  s_var.GetDimensions(v_9);
  uint v_10 = asuint(int(0));
  uint v_11 = asuint(int(8));
  uint v_12 = (v_10 + select((((v_11 != 0u) & (7u != 0u)) & (7u > (4294967295u / v_11))), 4294967295u, (v_11 * 7u)));
  uint v_13 = select((v_12 < v_10), 4294967295u, v_12);
  uint v_14 = (v_13 + 8u);
  bool v_15 = (select((v_14 < v_13), 4294967295u, v_14) <= (v_9 / 4u));
  m2.Store(s_var, (0u + (select(v_15, v_10, 0u) * 4u)), (select(v_15, v_11, 8u) * 4u), MatrixLayout::ColMajor);
}

[numthreads(32, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

