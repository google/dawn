// [hlsl-dxc] flags: --hlsl-shader-model 6.10
enable chromium_experimental_subgroup_matrix;
enable f16;
var<workgroup> out0 : array<u32, 1024>;
var<workgroup> out1 : array<vec2i, 1024>;
var<workgroup> out2 : array<vec3f, 1024>;
var<workgroup> out3 : array<vec4u, 1024>;
var<workgroup> out5 : array<vec2h, 1024>;
var<workgroup> out6 : array<vec3h, 1024>;
@compute @workgroup_size(64) fn main() {
  let m = subgroup_matrix_right<f32, 8, 8>();
  subgroupMatrixStore<row_major>(&out0, 0i, m, 16u);
  subgroupMatrixStore<row_major>(&out1, 0i, m, 16u);
  subgroupMatrixStore<row_major>(&out2, 0i, m, 16u);
  subgroupMatrixStore<row_major>(&out3, 0i, m, 16u);
  subgroupMatrixStore<row_major>(&out5, 0i, m, 16u);
  subgroupMatrixStore<row_major>(&out6, 0i, m, 16u);
}
