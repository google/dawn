SKIP: FAILED


enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_45de0c() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_left<f16, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_45de0c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMatrixStore/45de0c.wgsl:41:8 error: HLSL backend does not support extension 'chromium_experimental_subgroup_matrix'
enable chromium_experimental_subgroup_matrix;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
