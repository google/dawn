// [hlsl-dxc] flags: --hlsl-shader-model 6.10
enable chromium_experimental_subgroup_matrix;
enable f16;
@group(0) @binding(0) var<storage, read_write> out0 : array<u32, 1024>;
@group(0) @binding(1) var<storage, read_write> out1 : array<vec2i, 1024>;
@group(0) @binding(2) var<storage, read_write> out2 : array<vec3f, 1024>;
@group(0) @binding(3) var<storage, read_write> out3 : array<vec4u, 1024>;
@group(0) @binding(4) var<storage, read_write> out4 : array<f16, 1024>;
@group(0) @binding(5) var<storage, read_write> out5 : array<vec2h, 1024>;
@group(0) @binding(6) var<storage, read_write> out6 : array<vec3h, 1024>;
@compute @workgroup_size(64) fn main() {
  let m = subgroup_matrix_right<i8, 8, 8>();
  subgroupMatrixStore<row_major>(&out0, 0u, m, 16u);
  subgroupMatrixStore<row_major>(&out1, 0u, m, 16u);
  subgroupMatrixStore<row_major>(&out2, 0u, m, 16u);
  subgroupMatrixStore<row_major>(&out3, 0u, m, 16u);
  subgroupMatrixStore<row_major>(&out4, 0u, m, 16u);
  subgroupMatrixStore<row_major>(&out5, 0u, m, 16u);
  subgroupMatrixStore<row_major>(&out6, 0u, m, 16u);
}
