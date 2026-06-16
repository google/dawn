#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared float arg_0[1024];
Matrix_left_f32_8x8 subgroupMatrixLoad_d94ccb() {
  Matrix_left_f32_8x8 res = Matrix_left_f32_8x8::Load(arg_0, 1u, 8u, MatrixLayout::RowMajor);
  return res;
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
      arg_0[v_1] = 0.0f;
      {
        v = (v_1 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  subgroupMatrixLoad_d94ccb().Store(prevent_dce, 0u, 256u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

