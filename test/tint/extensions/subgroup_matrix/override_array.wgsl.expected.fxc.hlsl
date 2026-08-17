SKIP: FAILED

$B1: {  # root
  %s_var:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
  %o:u32 = override 1024u @id(0)
  %wg_var:ptr<workgroup, array<f32, %o>, read_write> = var undef
}

%main = @compute @workgroup_size(32i, 1i, 1i) func():void {
  $B2: {
    %5:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major> %s_var, 0i, 8i
    %m:subgroup_matrix_left<f32, 8, 8> = let %5
    %7:void = subgroupMatrixStore<row_major> %wg_var, 0i, %m, 8i
    %8:void = workgroupBarrier
    %9:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, col_major> %wg_var, 0i, 8i
    %m2:subgroup_matrix_right<f32, 8, 8> = let %9
    %11:void = subgroupMatrixStore<col_major> %s_var, 0i, %m2, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
