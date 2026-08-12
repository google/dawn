enable chromium_experimental_subgroup_matrix, subgroups;
enable f16;

var<workgroup> out0 : array<u32, 1024>;

var<workgroup> out1 : array<vec2i, 1024>;

var<workgroup> out2 : array<vec3f, 1024>;

var<workgroup> out3 : array<vec4u, 1024>;

var<workgroup> out4 : array<f16, 1024>;

var<workgroup> out5 : array<vec2h, 1024>;

var<workgroup> out6 : array<vec3h, 1024>;

@compute @workgroup_size(64)
fn main() {
  let m = subgroup_matrix_right<i8, 8, 8>();
  subgroupMatrixStore<col_major>(&(out0), 0i, m, 16u);
  subgroupMatrixStore<col_major>(&(out1), 0i, m, 16u);
  subgroupMatrixStore<col_major>(&(out2), 0i, m, 16u);
  subgroupMatrixStore<col_major>(&(out3), 0i, m, 16u);
  subgroupMatrixStore<col_major>(&(out4), 0i, m, 16u);
  subgroupMatrixStore<col_major>(&(out5), 0i, m, 16u);
  subgroupMatrixStore<col_major>(&(out6), 0i, m, 16u);
}
