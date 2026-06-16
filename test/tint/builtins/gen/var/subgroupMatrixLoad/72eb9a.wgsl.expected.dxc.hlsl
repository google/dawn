#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_i32_8x8 = Matrix<ComponentType::I32, 8, 8, MatrixUse::B, MatrixScope::Wave>;
struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared int arg_0[1024];
Matrix_right_i32_8x8 subgroupMatrixLoad_72eb9a() {
  uint arg_1 = 1u;
  uint arg_2 = 8u;
  uint v = arg_1;
  uint v_1 = max(arg_2, 8u);
  Matrix_right_i32_8x8 v_2 = Matrix_right_i32_8x8::Splat(int(0));
  if ((((v + (v_1 * 7u)) + 8u) <= 1024u)) {
    v_2 = Matrix_right_i32_8x8::Load(arg_0, v, v_1, MatrixLayout::RowMajor);
  }
  Matrix_right_i32_8x8 res = v_2;
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
      arg_0[v_4] = int(0);
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  subgroupMatrixLoad_72eb9a().Store(prevent_dce, 0u, 256u, MatrixLayout::RowMajor);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

