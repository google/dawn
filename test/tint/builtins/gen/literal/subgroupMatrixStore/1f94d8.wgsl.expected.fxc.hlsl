SKIP: FAILED

$B1: {  # root
  %arg_0:ptr<workgroup, array<f16, 1024>, read_write> = var undef
}

%subgroupMatrixStore_1f94d8 = func():void {
  $B2: {
    %3:subgroup_matrix_result<f16, 8, 8> = construct
    %4:void = subgroupMatrixStore<col_major> %arg_0, 1i, %3, 8u
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %6:void = call %subgroupMatrixStore_1f94d8
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
