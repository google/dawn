// [hlsl-dxc] flags: --hlsl-shader-model 6.10
enable chromium_experimental_subgroup_matrix;
enable f16;
var<workgroup> in0 : array<u32, 1024>;
var<workgroup> in1 : array<vec2i, 1024>;
var<workgroup> in2 : array<vec3f, 1024>;
var<workgroup> in3 : array<vec4u, 1024>;
var<workgroup> in4 : array<f16, 1024>;
var<workgroup> in5 : array<vec2h, 1024>;
var<workgroup> in6 : array<vec3h, 1024>;
@group(0) @binding(0) var<storage, read_write> out : array<u32>;
@compute @workgroup_size(64) fn main() {
  let m0 = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, row_major>(&in0, 0u, 16u);
  subgroupMatrixStore<col_major>(&out, 0, m0, 16);
  let m1 = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, row_major>(&in1, 0u, 16u);
  subgroupMatrixStore<col_major>(&out, 0, m1, 16);
  let m2 = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, row_major>(&in2, 0u, 16u);
  subgroupMatrixStore<col_major>(&out, 0, m2, 16);
  let m3 = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, row_major>(&in3, 0u, 16u);
  subgroupMatrixStore<col_major>(&out, 0, m3, 16);
  let m4 = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, row_major>(&in4, 0u, 16u);
  subgroupMatrixStore<col_major>(&out, 0, m4, 16);
  let m5 = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, row_major>(&in5, 0u, 16u);
  subgroupMatrixStore<col_major>(&out, 0, m5, 16);
  let m6 = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, row_major>(&in6, 0u, 16u);
  subgroupMatrixStore<col_major>(&out, 0, m6, 16);
}
