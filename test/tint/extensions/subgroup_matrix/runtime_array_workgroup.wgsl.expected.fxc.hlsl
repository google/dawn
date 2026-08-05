SKIP: FAILED

$B1: {  # root
  %v:ptr<workgroup, buffer<1024>, read_write> = var undef
}

%main = @compute @workgroup_size(32i, 1i, 1i) func():void {
  $B2: {
    %3:ptr<workgroup, array<f32>, read_write> = bufferView<array<f32>> %v, 0i
    %view:ptr<workgroup, array<f32>, read_write> = let %3
    %5:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, col_major> %view, 0i, 8i
    %m:subgroup_matrix_left<f32, 8, 8> = let %5
    %7:void = subgroupMatrixStore<row_major> %view, 0i, %m, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
