SKIP: FAILED

$B1: {  # root
  %in0:ptr<storage, array<u32, 1024>, read> = var undef @binding_point(0, 1)
  %in1:ptr<storage, array<vec2<i32>, 1024>, read> = var undef @binding_point(0, 2)
  %in2:ptr<storage, array<vec3<f32>, 1024>, read> = var undef @binding_point(0, 3)
  %in3:ptr<storage, array<vec4<u32>, 1024>, read> = var undef @binding_point(0, 4)
  %in4:ptr<storage, array<f16, 1024>, read> = var undef @binding_point(0, 5)
  %in5:ptr<storage, array<vec2<f16>, 1024>, read> = var undef @binding_point(0, 6)
  %in6:ptr<storage, array<vec3<f16>, 1024>, read> = var undef @binding_point(0, 7)
  %out:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64i, 1i, 1i) func():void {
  $B2: {
    %10:subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major> %in0, 0i, 16u
    %m0:subgroup_matrix_right<f16, 8, 8> = let %10
    %12:void = subgroupMatrixStore<col_major> %out, 0i, %m0, 16i
    %13:subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major> %in1, 0i, 16u
    %m1:subgroup_matrix_right<f16, 8, 8> = let %13
    %15:void = subgroupMatrixStore<col_major> %out, 0i, %m1, 16i
    %16:subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major> %in2, 0i, 16u
    %m2:subgroup_matrix_right<f16, 8, 8> = let %16
    %18:void = subgroupMatrixStore<col_major> %out, 0i, %m2, 16i
    %19:subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major> %in3, 0i, 16u
    %m3:subgroup_matrix_right<f16, 8, 8> = let %19
    %21:void = subgroupMatrixStore<col_major> %out, 0i, %m3, 16i
    %22:subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major> %in4, 0i, 16u
    %m4:subgroup_matrix_right<f16, 8, 8> = let %22
    %24:void = subgroupMatrixStore<col_major> %out, 0i, %m4, 16i
    %25:subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major> %in5, 0i, 16u
    %m5:subgroup_matrix_right<f16, 8, 8> = let %25
    %27:void = subgroupMatrixStore<col_major> %out, 0i, %m5, 16i
    %28:subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major> %in6, 0i, 16u
    %m6:subgroup_matrix_right<f16, 8, 8> = let %28
    %30:void = subgroupMatrixStore<col_major> %out, 0i, %m6, 16i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
