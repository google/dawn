SKIP: FAILED


enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_45de0c() {
  var arg_1 = 1u;
  var arg_2 = subgroup_matrix_left<f16, 8, 8>();
  const arg_3 = true;
  var arg_4 = 1u;
  subgroupMatrixStore(&(arg_0), arg_1, arg_2, arg_3, arg_4);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_45de0c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMatrixStore/45de0c.wgsl:41:8 error: HLSL backend does not support extension 'chromium_experimental_subgroup_matrix'
enable chromium_experimental_subgroup_matrix;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
