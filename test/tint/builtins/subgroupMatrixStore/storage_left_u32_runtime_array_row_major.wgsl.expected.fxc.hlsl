SKIP: FAILED

$B1: {  # root
  %out0:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
  %out1:ptr<storage, array<vec2<i32>>, read_write> = var undef @binding_point(0, 1)
  %out2:ptr<storage, array<vec3<f32>>, read_write> = var undef @binding_point(0, 2)
  %out3:ptr<storage, array<vec4<u32>>, read_write> = var undef @binding_point(0, 3)
  %out5:ptr<storage, array<vec2<f16>>, read_write> = var undef @binding_point(0, 5)
  %out6:ptr<storage, array<vec3<f16>>, read_write> = var undef @binding_point(0, 6)
}

%main = @compute @workgroup_size(64i, 1i, 1i) func():void {
  $B2: {
    %8:subgroup_matrix_left<u32, 8, 8> = construct
    %m:subgroup_matrix_left<u32, 8, 8> = let %8
    %10:void = subgroupMatrixStore<row_major> %out0, 0i, %m, 16u
    %11:void = subgroupMatrixStore<row_major> %out1, 0i, %m, 16u
    %12:void = subgroupMatrixStore<row_major> %out2, 0i, %m, 16u
    %13:void = subgroupMatrixStore<row_major> %out3, 0i, %m, 16u
    %14:void = subgroupMatrixStore<row_major> %out5, 0i, %m, 16u
    %15:void = subgroupMatrixStore<row_major> %out6, 0i, %m, 16u
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
