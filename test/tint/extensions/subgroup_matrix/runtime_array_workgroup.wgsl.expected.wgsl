enable chromium_experimental_subgroup_matrix;

var<workgroup> v : buffer<1024>;

@compute @workgroup_size(32)
fn main() {
  let view = bufferView<array<f32>>(&(v), 0);
  let m = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, col_major>(view, 0, 8);
  subgroupMatrixStore<row_major>(view, 0, m, 8);
}
