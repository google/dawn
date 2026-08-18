// [hlsl-dxc] flags: --hlsl-shader-model 6.10

enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0)
var<storage, read_write> ibuffer: array<i32>;
@group(0) @binding(1)
var<storage, read_write> ubuffer: array<u32>;

@compute @workgroup_size(64)
fn main() {
  // Zero init.
  subgroupMatrixStore<row_major>(&ibuffer, 0, subgroup_matrix_left<i8, 8, 8>(), 64);
  subgroupMatrixStore<row_major>(&ubuffer, 0, subgroup_matrix_right<u8, 8, 8>(), 64);

  // Non-zero init.
  subgroupMatrixStore<row_major>(&ibuffer, 0, subgroup_matrix_left<i8, 8, 8>(-42), 64);
  subgroupMatrixStore<row_major>(&ubuffer, 0, subgroup_matrix_right<u8, 8, 8>(42), 64);
}
