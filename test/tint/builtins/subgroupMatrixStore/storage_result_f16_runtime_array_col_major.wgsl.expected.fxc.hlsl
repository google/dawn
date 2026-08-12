SKIP: FAILED

$B1: {  # root
  %out0:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
  %out1:ptr<storage, array<vec2<i32>>, read_write> = var undef @binding_point(0, 1)
  %out2:ptr<storage, array<vec3<f32>>, read_write> = var undef @binding_point(0, 2)
  %out3:ptr<storage, array<vec4<u32>>, read_write> = var undef @binding_point(0, 3)
  %out4:ptr<storage, array<f16>, read_write> = var undef @binding_point(0, 4)
  %out5:ptr<storage, array<vec2<f16>>, read_write> = var undef @binding_point(0, 5)
  %out6:ptr<storage, array<vec3<f16>>, read_write> = var undef @binding_point(0, 6)
}

%main = @compute @workgroup_size(64i, 1i, 1i) func():void {
  $B2: {
    %9:subgroup_matrix_result<f16, 8, 8> = construct
    %m:subgroup_matrix_result<f16, 8, 8> = let %9
    %11:void = subgroupMatrixStore<col_major> %out0, 0u, %m, 16u
    %12:void = subgroupMatrixStore<col_major> %out1, 0u, %m, 16u
    %13:void = subgroupMatrixStore<col_major> %out2, 0u, %m, 16u
    %14:void = subgroupMatrixStore<col_major> %out3, 0u, %m, 16u
    %15:void = subgroupMatrixStore<col_major> %out4, 0u, %m, 16u
    %16:void = subgroupMatrixStore<col_major> %out5, 0u, %m, 16u
    %17:void = subgroupMatrixStore<col_major> %out6, 0u, %m, 16u
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
