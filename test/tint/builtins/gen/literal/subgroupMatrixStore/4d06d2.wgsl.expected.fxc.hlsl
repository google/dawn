SKIP: FAILED

SB_RW = struct @align(4) {
  arg_0:array<u32, 1024> @offset(0)
}

$B1: {  # root
  %sb_rw:ptr<storage, SB_RW, read_write> = var undef @binding_point(0, 0)
}

%subgroupMatrixStore_4d06d2 = func():void {
  $B2: {
    %3:ptr<storage, array<u32, 1024>, read_write> = access %sb_rw, 0u
    %4:subgroup_matrix_left<u8, 8, 8> = construct
    %5:void = subgroupMatrixStore<row_major> %3, 1u, %4, 8i
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %7:void = call %subgroupMatrixStore_4d06d2
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
