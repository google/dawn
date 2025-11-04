enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f16, 1024>;

fn subgroupMatrixScalarAdd_9dbd1a() -> subgroup_matrix_right<f16, 8, 8> {
  var arg_0 = subgroup_matrix_right<f16, 8, 8>();
  var arg_1 = 8.0h;
  var res : subgroup_matrix_right<f16, 8, 8> = subgroupMatrixScalarAdd(arg_0, arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixScalarAdd_9dbd1a(), false, 64);
}
