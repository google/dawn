enable chromium_experimental_subgroup_matrix, subgroups;
enable f16;

@group(0) @binding(1) var<storage> in0 : array<u32, 1024>;

@group(0) @binding(2) var<storage> in1 : array<vec2i, 1024>;

@group(0) @binding(3) var<storage> in2 : array<vec3f, 1024>;

@group(0) @binding(4) var<storage> in3 : array<vec4u, 1024>;

@group(0) @binding(6) var<storage> in5 : array<vec2h, 1024>;

@group(0) @binding(7) var<storage> in6 : array<vec3h, 1024>;

@group(0) @binding(0) var<storage, read_write> out : array<u32>;

@compute @workgroup_size(64)
fn main() {
  let m0 = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(in0), 0u, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m0, 16);
  let m1 = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(in1), 0u, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m1, 16);
  let m2 = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(in2), 0u, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m2, 16);
  let m3 = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(in3), 0u, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m3, 16);
  let m5 = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(in5), 0u, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m5, 16);
  let m6 = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(in6), 0u, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m6, 16);
}
