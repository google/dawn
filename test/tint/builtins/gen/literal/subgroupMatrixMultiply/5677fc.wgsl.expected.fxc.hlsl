SKIP: FAILED

$B1: {  # root
  %prevent_dce:ptr<storage, array<f32, 1024>, read_write> = var undef @binding_point(0, 0)
}

%subgroupMatrixMultiply_5677fc = func():subgroup_matrix_result<f32, 8, 8> {
  $B2: {
    %3:subgroup_matrix_left<f16, 8, 8> = construct
    %4:subgroup_matrix_right<f16, 8, 8> = construct
    %5:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiply<f32> %3, %4
    %res:ptr<function, subgroup_matrix_result<f32, 8, 8>, read_write> = var %5
    %7:subgroup_matrix_result<f32, 8, 8> = load %res
    ret %7
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %9:subgroup_matrix_result<f32, 8, 8> = call %subgroupMatrixMultiply_5677fc
    %10:void = subgroupMatrixStore %prevent_dce, 0i, %9, false, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
