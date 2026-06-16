SKIP: FAILED

$B1: {  # root
  %arg_0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
}

%subgroupMatrixStore_3c8f78 = func():void {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var 1u
    %4:subgroup_matrix_left<u8, 8, 8> = construct
    %arg_2:ptr<function, subgroup_matrix_left<u8, 8, 8>, read_write> = var %4
    %arg_3:ptr<function, u32, read_write> = var 8u
    %7:u32 = load %arg_1
    %8:subgroup_matrix_left<u8, 8, 8> = load %arg_2
    %9:u32 = load %arg_3
    %10:void = subgroupMatrixStore %arg_0, %7, %8, false, %9
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %12:void = call %subgroupMatrixStore_3c8f78
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
