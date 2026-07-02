SKIP: FAILED

$B1: {  # root
  %arg_0:ptr<workgroup, array<i32, 1024>, read_write> = var undef
}

%subgroupMatrixStore_02622f = func():void {
  $B2: {
    %3:subgroup_matrix_left<i32, 8, 8> = construct
    %4:void = subgroupMatrixStore %arg_0, 1i, %3, true, 8i
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %6:void = call %subgroupMatrixStore_02622f
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
