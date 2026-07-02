SKIP: FAILED

$B1: {  # root
  %prevent_dce:ptr<storage, array<u32, 1024>, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
}

%subgroupMatrixLoad_324ac8 = func():subgroup_matrix_result<u8, 8, 8> {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var 1u
    %arg_3:ptr<function, i32, read_write> = var 8i
    %6:u32 = load %arg_1
    %7:i32 = load %arg_3
    %8:subgroup_matrix_result<u8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<u8, 8, 8>> %arg_0, %6, true, %7
    %res:ptr<function, subgroup_matrix_result<u8, 8, 8>, read_write> = var %8
    %10:subgroup_matrix_result<u8, 8, 8> = load %res
    ret %10
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %12:subgroup_matrix_result<u8, 8, 8> = call %subgroupMatrixLoad_324ac8
    %13:void = subgroupMatrixStore %prevent_dce, 0i, %12, false, 8i
    ret
  }
}
Failed to generate: subgroup matrices support requires DXC with HLSL 2021

tint executable returned error: exit status 1
