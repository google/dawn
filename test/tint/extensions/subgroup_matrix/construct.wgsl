enable chromium_experimental_subgroup_matrix;

struct S {
  l: subgroup_matrix_left<f32, 8, 8>,
  r: subgroup_matrix_right<f32, 8, 8>,
}

struct S_Nested {
  s: S,
}

@group(0) @binding(0)
var<storage, read_write> buffer: array<f32>;

@compute @workgroup_size(64)
fn main() {
  // Zero init.
  subgroupMatrixStore(&buffer, 0, subgroup_matrix_left<f32, 8, 8>(), false, 64);
  subgroupMatrixStore(&buffer, 0, array<subgroup_matrix_left<f32, 8, 8>, 4>()[1], false, 64);
  subgroupMatrixStore(&buffer, 0, array<array<subgroup_matrix_left<f32, 8, 8>, 4>, 4>()[2][3], false, 64);
  subgroupMatrixStore(&buffer, 0, S().l, false, 64);
  subgroupMatrixStore(&buffer, 0, S_Nested().s.r, false, 64);

  // Non-zero init.
  subgroupMatrixStore(&buffer, 0, subgroup_matrix_left<f32, 8, 8>(42), false, 64);
  subgroupMatrixStore(&buffer, 0, array<subgroup_matrix_left<f32, 8, 8>, 2>(
                                    subgroup_matrix_left<f32, 8, 8>(42),
                                    subgroup_matrix_left<f32, 8, 8>(100),
                                  )[1], false, 64);
  subgroupMatrixStore(&buffer, 0, array<array<subgroup_matrix_left<f32, 8, 8>, 2>, 2>(
                                    array<subgroup_matrix_left<f32, 8, 8>, 2>(
                                      subgroup_matrix_left<f32, 8, 8>(42),
                                      subgroup_matrix_left<f32, 8, 8>(100),
                                    ),
                                    array<subgroup_matrix_left<f32, 8, 8>, 2>(
                                      subgroup_matrix_left<f32, 8, 8>(-7),
                                      subgroup_matrix_left<f32, 8, 8>(-42),
                                    )
                                  )[1][0], false, 64);
  subgroupMatrixStore(&buffer, 0, S(
                                    subgroup_matrix_left<f32, 8, 8>(42),
                                    subgroup_matrix_right<f32, 8, 8>(100),
                                  ).l, false, 64);
  subgroupMatrixStore(&buffer, 0, S_Nested(
                                    S(
                                      subgroup_matrix_left<f32, 8, 8>(42),
                                      subgroup_matrix_right<f32, 8, 8>(100),
                                    ),
                                  ).s.r, false, 64);
}
