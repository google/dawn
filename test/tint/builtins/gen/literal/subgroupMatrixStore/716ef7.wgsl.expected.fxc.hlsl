SKIP: INVALID


enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_716ef7() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<u32, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_716ef7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMatrixStore/716ef7.wgsl:41:8 error: HLSL backend does not support extension 'chromium_experimental_subgroup_matrix'
enable chromium_experimental_subgroup_matrix;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
