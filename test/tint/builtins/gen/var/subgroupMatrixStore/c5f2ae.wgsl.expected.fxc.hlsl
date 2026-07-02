SKIP: FAILED

$B1: {  # root
  %arg_0:ptr<workgroup, array<f16, 1024>, read_write> = var undef
}

%subgroupMatrixStore_c5f2ae = func():void {
  $B2: {
    %arg_1:ptr<function, i32, read_write> = var 1i
    %4:subgroup_matrix_right<f16, 8, 8> = construct
    %arg_2:ptr<function, subgroup_matrix_right<f16, 8, 8>, read_write> = var %4
    %arg_3:ptr<function, i32, read_write> = var 8i
    %7:i32 = load %arg_1
    %8:subgroup_matrix_right<f16, 8, 8> = load %arg_2
    %9:i32 = load %arg_3
    %10:void = subgroupMatrixStore<row_major> %arg_0, %7, %8, %9
    ret
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %12:void = call %subgroupMatrixStore_c5f2ae
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
