SKIP: FAILED


enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_840822() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_result<f16, 8, 8>(), true, 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_840822();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMatrixStore/840822.wgsl:41:8 error: HLSL backend does not support extension 'chromium_experimental_subgroup_matrix'
enable chromium_experimental_subgroup_matrix;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
