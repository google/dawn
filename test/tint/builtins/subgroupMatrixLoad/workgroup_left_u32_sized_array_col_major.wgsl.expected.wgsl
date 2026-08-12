enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> in0 : array<u32, 1024>;

var<workgroup> in1 : array<vec2i, 1024>;

var<workgroup> in2 : array<vec3f, 1024>;

var<workgroup> in3 : array<vec4u, 1024>;

var<workgroup> in5 : array<vec2h, 1024>;

var<workgroup> in6 : array<vec3h, 1024>;

@group(0) @binding(0) var<storage, read_write> out : array<u32>;

@compute @workgroup_size(64)
fn main() {
  let m0 = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, col_major>(&(in0), 0i, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m0, 16);
  let m1 = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, col_major>(&(in1), 0i, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m1, 16);
  let m2 = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, col_major>(&(in2), 0i, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m2, 16);
  let m3 = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, col_major>(&(in3), 0i, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m3, 16);
  let m5 = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, col_major>(&(in5), 0i, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m5, 16);
  let m6 = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, col_major>(&(in6), 0i, 16u);
  subgroupMatrixStore<col_major>(&(out), 0, m6, 16);
}
