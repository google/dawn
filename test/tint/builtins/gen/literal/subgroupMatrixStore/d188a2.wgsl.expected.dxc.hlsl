#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint arg_0[1024];
void subgroupMatrixStore_d188a2() {
  Matrix_left_u32_8x8::Splat(0u).Store(arg_0, 1u, 8u, MatrixLayout::ColMajor);
}

void compute_main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 1024u)) {
        break;
      }
      arg_0[v_1] = 0u;
      {
        v = (v_1 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  subgroupMatrixStore_d188a2();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

