SKIP: INVALID


enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixStore_c4a7ce() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<f32, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_c4a7ce();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMatrixStore/c4a7ce.wgsl:41:8 error: HLSL backend does not support extension 'chromium_experimental_subgroup_matrix'
enable chromium_experimental_subgroup_matrix;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
