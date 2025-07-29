// flags: --disable-robustness true
// This test is to specifically check for loop condition reconstruction.
// See crbug.com/426458025

struct Scalars {
  f0 : vec4<f32>,
  i1 : vec4<i32>,
  i2 : vec4<i32>,
  i3 : vec4<i32>,
};
@group(0) @binding(3) var<uniform> U: Scalars;
@group(0) @binding(1) var dst_image2d : texture_storage_2d<rgba32uint, write>;
@group(0) @binding(2) var src_image2d : texture_2d<f32>;
var<workgroup> outputs : array<array<vec4<u32>, 32>, 8>;
@compute @workgroup_size(32, 1, 8)

fn main(@builtin(local_invocation_id) lid : vec3<u32>) {
  let init = i32(lid.z);
  for (var S = init; S < U.i3.x; S += 8i) {
  }

  for (var s_group = 0i; s_group < U.i3.z; s_group += 8i) {
    outputs[lid.z][lid.x] = vec4<u32>(textureLoad(src_image2d, vec2u(u32(U.i3.x)), 0));
    workgroupBarrier();
    var result = outputs[lid.z][lid.x];;
    textureStore(dst_image2d, vec2u(u32(U.i3.x)), result);
  }
}