SKIP: FAILED

SB_RW = struct @align(4) {
  arg_0:array<u32, 1024> @offset(0)
}

$B1: {  # root
  %sb_rw:ptr<storage, SB_RW, read_write> = var undef @binding_point(0, 0)
}

%subgroupMatrixStore_a92a15 = func():void {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var 1u
    %4:subgroup_matrix_right<u8, 8, 8> = construct
    %arg_2:ptr<function, subgroup_matrix_right<u8, 8, 8>, read_write> = var %4
    %arg_4:ptr<function, i32, read_write> = var 8i
    %7:ptr<storage, array<u32, 1024>, read_write> = access %sb_rw, 0u
    %8:u32 = load %arg_1
    %9:subgroup_matrix_right<u8, 8, 8> = load %arg_2
    %10:i32 = load %arg_4
    %11:void = subgroupMatrixStore %7, %8, %9, true, %10
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %13:void = call %subgroupMatrixStore_a92a15
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
