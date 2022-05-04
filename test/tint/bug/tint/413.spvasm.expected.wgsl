@group(0) @binding(0) var Src : texture_2d<u32>;

@group(0) @binding(1) var Dst : texture_storage_2d<r32uint, write>;

fn main_1() {
  var srcValue : vec4<u32>;
  let x_18 : vec4<u32> = textureLoad(Src, vec2<i32>(0i, 0i), 0i);
  srcValue = x_18;
  let x_22 : u32 = srcValue.x;
  srcValue.x = (x_22 + bitcast<u32>(1i));
  let x_27 : vec4<u32> = srcValue;
  textureStore(Dst, vec2<i32>(0i, 0i), x_27);
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
