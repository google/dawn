SKIP: FAILED

$B1: {  # root
  %prevent_dce:ptr<storage, array<u32, 1024>, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
}

%subgroupMatrixLoad_7adcb2 = func():subgroup_matrix_left<u8, 8, 8> {
  $B2: {
    %4:subgroup_matrix_left<u8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>> %arg_0, 1i, true, 8i
    %res:ptr<function, subgroup_matrix_left<u8, 8, 8>, read_write> = var %4
    %6:subgroup_matrix_left<u8, 8, 8> = load %res
    ret %6
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %8:subgroup_matrix_left<u8, 8, 8> = call %subgroupMatrixLoad_7adcb2
    %9:void = subgroupMatrixStore %prevent_dce, 0i, %8, false, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
