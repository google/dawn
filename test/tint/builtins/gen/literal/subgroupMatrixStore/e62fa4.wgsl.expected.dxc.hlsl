#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_f16_8x8 = Matrix<ComponentType::F16, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float16_t arg_0[1024];
void subgroupMatrixStore_e62fa4() {
  Matrix_result_f16_8x8::Splat(float16_t(0.0h)).Store(arg_0, 1u, 8u, MatrixLayout::ColMajor);
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
      arg_0[v_1] = float16_t(0.0h);
      {
        v = (v_1 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  subgroupMatrixStore_e62fa4();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

