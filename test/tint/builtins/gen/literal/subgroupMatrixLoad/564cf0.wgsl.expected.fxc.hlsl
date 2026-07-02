SKIP: FAILED

SB_RW = struct @align(4) {
  arg_0:array<i32, 1024> @offset(0)
}

$B1: {  # root
  %prevent_dce:ptr<storage, array<i32, 1024>, read_write> = var undef @binding_point(0, 0)
  %sb_rw:ptr<storage, SB_RW, read_write> = var undef @binding_point(0, 1)
}

%subgroupMatrixLoad_564cf0 = func():subgroup_matrix_right<i32, 8, 8> {
  $B2: {
    %4:ptr<storage, array<i32, 1024>, read_write> = access %sb_rw, 0u
    %5:subgroup_matrix_right<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major> %4, 1u, 8i
    %res:ptr<function, subgroup_matrix_right<i32, 8, 8>, read_write> = var %5
    %7:subgroup_matrix_right<i32, 8, 8> = load %res
    ret %7
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %9:subgroup_matrix_right<i32, 8, 8> = call %subgroupMatrixLoad_564cf0
    %10:void = subgroupMatrixStore %prevent_dce, 0i, %9, false, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
