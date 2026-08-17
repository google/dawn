// [hlsl-dxc] flags: --hlsl-shader-model 6.10

enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> s_var : array<f32>;

override o : u32 = 1024;

var<workgroup> wg_var : array<f32, o>;

@compute @workgroup_size(32)
fn main() {
  let m = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&s_var, 0, 8);
  subgroupMatrixStore<row_major>(&wg_var, 0, m, 8);
  workgroupBarrier();
  let m2 = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, col_major>(&wg_var, 0, 8);
  subgroupMatrixStore<col_major>(&s_var, 0, m2, 8);
}
