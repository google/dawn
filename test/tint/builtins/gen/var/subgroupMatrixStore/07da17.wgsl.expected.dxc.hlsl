#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;
struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint arg_0[1024];
void subgroupMatrixStore_07da17() {
  uint arg_1 = 1u;
  Matrix_right_u32_8x8 arg_2 = Matrix_right_u32_8x8::Splat(0u);
  uint arg_3 = 8u;
  uint v = arg_1;
  Matrix_right_u32_8x8 v_1 = arg_2;
  uint v_2 = max(arg_3, 8u);
  if ((((v + (v_2 * 7u)) + 8u) <= 1024u)) {
    v_1.Store(arg_0, v, v_2, MatrixLayout::ColMajor);
  }
}

void compute_main_inner(uint tint_local_index) {
  {
    uint v_3 = 0u;
    v_3 = tint_local_index;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 1024u)) {
        break;
      }
      arg_0[v_4] = 0u;
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  subgroupMatrixStore_07da17();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

