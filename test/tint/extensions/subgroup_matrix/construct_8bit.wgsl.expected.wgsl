enable chromium_experimental_subgroup_matrix, subgroups;

@group(0) @binding(0) var<storage, read_write> ibuffer : array<i32>;

@group(0) @binding(1) var<storage, read_write> ubuffer : array<u32>;

@compute @workgroup_size(64)
fn main() {
  subgroupMatrixStore<row_major>(&(ibuffer), 0, subgroup_matrix_left<i8, 8, 8>(), 64);
  subgroupMatrixStore<row_major>(&(ubuffer), 0, subgroup_matrix_right<u8, 8, 8>(), 64);
  subgroupMatrixStore<row_major>(&(ibuffer), 0, subgroup_matrix_left<i8, 8, 8>(-(42)), 64);
  subgroupMatrixStore<row_major>(&(ubuffer), 0, subgroup_matrix_right<u8, 8, 8>(42), 64);
}
