enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<u32, 1024>;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixLoad_f406f6() -> subgroup_matrix_right<u32, 8, 8> {
  var res : subgroup_matrix_right<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<u32, 8, 8>>(&(arg_0), 1u, true, 1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_f406f6(), false, 64);
}
