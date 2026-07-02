SKIP: FAILED

$B1: {  # root
  %arg_0:ptr<workgroup, array<f32, 1024>, read_write> = var undef
}

%subgroupMatrixStore_cd1b20 = func():void {
  $B2: {
    %3:subgroup_matrix_right<f32, 8, 8> = construct
    %4:void = subgroupMatrixStore<col_major> %arg_0, 1u, %3, 8u
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %6:void = call %subgroupMatrixStore_cd1b20
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
