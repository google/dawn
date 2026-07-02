SKIP: FAILED

SB_RW = struct @align(2) {
  arg_0:array<f16, 1024> @offset(0)
}

$B1: {  # root
  %prevent_dce:ptr<storage, array<f16, 1024>, read_write> = var undef @binding_point(0, 0)
  %sb_rw:ptr<storage, SB_RW, read_write> = var undef @binding_point(0, 1)
}

%subgroupMatrixLoad_6e8c0e = func():subgroup_matrix_result<f16, 8, 8> {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var 1u
    %arg_2:ptr<function, i32, read_write> = var 8i
    %6:ptr<storage, array<f16, 1024>, read_write> = access %sb_rw, 0u
    %7:u32 = load %arg_1
    %8:i32 = load %arg_2
    %9:subgroup_matrix_result<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major> %6, %7, %8
    %res:ptr<function, subgroup_matrix_result<f16, 8, 8>, read_write> = var %9
    %11:subgroup_matrix_result<f16, 8, 8> = load %res
    ret %11
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %13:subgroup_matrix_result<f16, 8, 8> = call %subgroupMatrixLoad_6e8c0e
    %14:void = subgroupMatrixStore %prevent_dce, 0i, %13, false, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
