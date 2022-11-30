@group(0) @binding(0) var<uniform> a : array<mat2x2<f32>, 4>;

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_2 = &((*(p_a))[2]);
  let p_a_2_1 = &((*(p_a_2))[1]);
  let l_a : array<mat2x2<f32>, 4> = *(p_a);
  let l_a_i : mat2x2<f32> = *(p_a_2);
  let l_a_i_i : vec2<f32> = *(p_a_2_1);
}
