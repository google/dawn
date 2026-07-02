SKIP: FAILED

SB_RW = struct @align(4) {
  arg_0:array<f32, 1024> @offset(0)
}

$B1: {  # root
  %sb_rw:ptr<storage, SB_RW, read_write> = var undef @binding_point(0, 0)
}

%subgroupMatrixStore_2e1692 = func():void {
  $B2: {
    %3:ptr<storage, array<f32, 1024>, read_write> = access %sb_rw, 0u
    %4:subgroup_matrix_right<f32, 8, 8> = construct
    %5:void = subgroupMatrixStore %3, 1u, %4, true, 8i
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %7:void = call %subgroupMatrixStore_2e1692
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
