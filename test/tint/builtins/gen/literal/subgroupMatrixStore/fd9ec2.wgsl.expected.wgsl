enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_fd9ec2() {
  subgroupMatrixStore(&(arg_0), 1i, subgroup_matrix_left<u32, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_fd9ec2();
}
