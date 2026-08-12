enable chromium_experimental_subgroup_matrix, subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> out0 : array<u32>;

@group(0) @binding(1) var<storage, read_write> out1 : array<vec2i>;

@group(0) @binding(2) var<storage, read_write> out2 : array<vec3f>;

@group(0) @binding(3) var<storage, read_write> out3 : array<vec4u>;

@group(0) @binding(5) var<storage, read_write> out5 : array<vec2h>;

@group(0) @binding(6) var<storage, read_write> out6 : array<vec3h>;

@compute @workgroup_size(64)
fn main() {
  let m = subgroup_matrix_left<u32, 8, 8>();
  subgroupMatrixStore<row_major>(&(out0), 0i, m, 16u);
  subgroupMatrixStore<row_major>(&(out1), 0i, m, 16u);
  subgroupMatrixStore<row_major>(&(out2), 0i, m, 16u);
  subgroupMatrixStore<row_major>(&(out3), 0i, m, 16u);
  subgroupMatrixStore<row_major>(&(out5), 0i, m, 16u);
  subgroupMatrixStore<row_major>(&(out6), 0i, m, 16u);
}
