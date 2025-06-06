SKIP: FAILED


enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixStore_a3ed32() {
  var arg_1 = 1u;
  var arg_2 = subgroup_matrix_result<f32, 8, 8>();
  const arg_3 = true;
  var arg_4 = 8u;
  subgroupMatrixStore(&(arg_0), arg_1, arg_2, arg_3, arg_4);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_a3ed32();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMatrixStore/a3ed32.wgsl:41:8 error: HLSL backend does not support extension 'chromium_experimental_subgroup_matrix'
enable chromium_experimental_subgroup_matrix;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
