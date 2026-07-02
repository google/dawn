SKIP: FAILED

SB_RO = struct @align(4) {
  arg_0:array<i32, 1024> @offset(0)
}

$B1: {  # root
  %prevent_dce:ptr<storage, array<i32, 1024>, read_write> = var undef @binding_point(0, 0)
  %sb_ro:ptr<storage, SB_RO, read> = var undef @binding_point(0, 1)
}

%subgroupMatrixLoad_62cea5 = func():subgroup_matrix_result<i8, 8, 8> {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var 1u
    %arg_2:ptr<function, i32, read_write> = var 8i
    %6:ptr<storage, array<i32, 1024>, read> = access %sb_ro, 0u
    %7:u32 = load %arg_1
    %8:i32 = load %arg_2
    %9:subgroup_matrix_result<i8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i8, 8, 8>, col_major> %6, %7, %8
    %res:ptr<function, subgroup_matrix_result<i8, 8, 8>, read_write> = var %9
    %11:subgroup_matrix_result<i8, 8, 8> = load %res
    ret %11
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %13:subgroup_matrix_result<i8, 8, 8> = call %subgroupMatrixLoad_62cea5
    %14:void = subgroupMatrixStore %prevent_dce, 0i, %13, false, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
