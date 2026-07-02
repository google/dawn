#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_result_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::Accumulator, MatrixScope::Wave>;
struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared uint arg_0[1024];
Matrix_result_u32_8x8 subgroupMatrixLoad_05c9c1() {
  uint arg_1 = 1u;
  int arg_2 = int(8);
  uint v = arg_1;
  uint v_1 = max(asuint(arg_2), 8u);
  Matrix_result_u32_8x8 v_2 = Matrix_result_u32_8x8::Splat(0u);
  if ((((v + (v_1 * 7u)) + 8u) <= 1024u)) {
    v_2 = Matrix_result_u32_8x8::Load(arg_0, v, v_1, MatrixLayout::RowMajor);
  }
  Matrix_result_u32_8x8 res = v_2;
  return res;
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
  subgroupMatrixLoad_05c9c1().Store(prevent_dce, 0u, 32u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

