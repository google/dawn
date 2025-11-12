enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> ibuffer : array<i32>;

@group(0) @binding(1) var<storage, read_write> ubuffer : array<u32>;

@compute @workgroup_size(64)
fn main() {
  subgroupMatrixStore(&(ibuffer), 0, subgroup_matrix_left<i8, 8, 8>(), false, 64);
  subgroupMatrixStore(&(ubuffer), 0, subgroup_matrix_right<u8, 8, 8>(), false, 64);
  subgroupMatrixStore(&(ibuffer), 0, subgroup_matrix_left<i8, 8, 8>(-(42)), false, 64);
  subgroupMatrixStore(&(ubuffer), 0, subgroup_matrix_right<u8, 8, 8>(42), false, 64);
}
