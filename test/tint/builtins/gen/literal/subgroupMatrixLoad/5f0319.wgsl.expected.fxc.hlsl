SKIP: FAILED

SB_RO = struct @align(4) {
  arg_0:array<i32, 1024> @offset(0)
}

$B1: {  # root
  %prevent_dce:ptr<storage, array<i32, 1024>, read_write> = var undef @binding_point(0, 0)
  %sb_ro:ptr<storage, SB_RO, read> = var undef @binding_point(0, 1)
}

%subgroupMatrixLoad_5f0319 = func():subgroup_matrix_result<i32, 8, 8> {
  $B2: {
    %4:ptr<storage, array<i32, 1024>, read> = access %sb_ro, 0u
    %5:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i32, 8, 8>, row_major> %4, 1i, 8u
    %res:ptr<function, subgroup_matrix_result<i32, 8, 8>, read_write> = var %5
    %7:subgroup_matrix_result<i32, 8, 8> = load %res
    ret %7
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %9:subgroup_matrix_result<i32, 8, 8> = call %subgroupMatrixLoad_5f0319
    %10:void = subgroupMatrixStore %prevent_dce, 0i, %9, false, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
