SKIP: FAILED

$B1: {  # root
  %out0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
  %out1:ptr<workgroup, array<vec2<i32>, 1024>, read_write> = var undef
  %out2:ptr<workgroup, array<vec3<f32>, 1024>, read_write> = var undef
  %out3:ptr<workgroup, array<vec4<u32>, 1024>, read_write> = var undef
  %out5:ptr<workgroup, array<vec2<f16>, 1024>, read_write> = var undef
  %out6:ptr<workgroup, array<vec3<f16>, 1024>, read_write> = var undef
}

%main = @compute @workgroup_size(64i, 1i, 1i) func():void {
  $B2: {
    %8:subgroup_matrix_result<i32, 8, 8> = construct
    %m:subgroup_matrix_result<i32, 8, 8> = let %8
    %10:void = subgroupMatrixStore<row_major> %out0, 0u, %m, 16u
    %11:void = subgroupMatrixStore<row_major> %out1, 0u, %m, 16u
    %12:void = subgroupMatrixStore<row_major> %out2, 0u, %m, 16u
    %13:void = subgroupMatrixStore<row_major> %out3, 0u, %m, 16u
    %14:void = subgroupMatrixStore<row_major> %out5, 0u, %m, 16u
    %15:void = subgroupMatrixStore<row_major> %out6, 0u, %m, 16u
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
