SKIP: FAILED

$B1: {  # root
  %in0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
  %in1:ptr<workgroup, array<vec2<i32>, 1024>, read_write> = var undef
  %in2:ptr<workgroup, array<vec3<f32>, 1024>, read_write> = var undef
  %in3:ptr<workgroup, array<vec4<u32>, 1024>, read_write> = var undef
  %in5:ptr<workgroup, array<vec2<f16>, 1024>, read_write> = var undef
  %in6:ptr<workgroup, array<vec3<f16>, 1024>, read_write> = var undef
  %out:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64i, 1i, 1i) func():void {
  $B2: {
    %9:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i32, 8, 8>, row_major> %in0, 0u, 16u
    %m0:subgroup_matrix_result<i32, 8, 8> = let %9
    %11:void = subgroupMatrixStore<col_major> %out, 0i, %m0, 16i
    %12:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i32, 8, 8>, row_major> %in1, 0u, 16u
    %m1:subgroup_matrix_result<i32, 8, 8> = let %12
    %14:void = subgroupMatrixStore<col_major> %out, 0i, %m1, 16i
    %15:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i32, 8, 8>, row_major> %in2, 0u, 16u
    %m2:subgroup_matrix_result<i32, 8, 8> = let %15
    %17:void = subgroupMatrixStore<col_major> %out, 0i, %m2, 16i
    %18:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i32, 8, 8>, row_major> %in3, 0u, 16u
    %m3:subgroup_matrix_result<i32, 8, 8> = let %18
    %20:void = subgroupMatrixStore<col_major> %out, 0i, %m3, 16i
    %21:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i32, 8, 8>, row_major> %in5, 0u, 16u
    %m5:subgroup_matrix_result<i32, 8, 8> = let %21
    %23:void = subgroupMatrixStore<col_major> %out, 0i, %m5, 16i
    %24:subgroup_matrix_result<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i32, 8, 8>, row_major> %in6, 0u, 16u
    %m6:subgroup_matrix_result<i32, 8, 8> = let %24
    %26:void = subgroupMatrixStore<col_major> %out, 0i, %m6, 16i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
