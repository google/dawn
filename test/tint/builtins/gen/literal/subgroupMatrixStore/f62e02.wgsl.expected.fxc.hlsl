SKIP: FAILED

SB_RW = struct @align(2) {
  arg_0:array<f16, 1024> @offset(0)
}

$B1: {  # root
  %sb_rw:ptr<storage, SB_RW, read_write> = var undef @binding_point(0, 0)
}

%subgroupMatrixStore_f62e02 = func():void {
  $B2: {
    %3:ptr<storage, array<f16, 1024>, read_write> = access %sb_rw, 0u
    %4:subgroup_matrix_result<f16, 8, 8> = construct
    %5:void = subgroupMatrixStore %3, 1u, %4, true, 8i
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %7:void = call %subgroupMatrixStore_f62e02
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
