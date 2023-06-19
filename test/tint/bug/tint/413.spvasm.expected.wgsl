@group(0) @binding(0) var Src : texture_2d<u32>;

@group(0) @binding(1) var Dst : texture_storage_2d<r32uint, write>;

fn main_1() {
  var srcValue : vec4u;
  let x_18 = textureLoad(Src, vec2i(), 0i);
  srcValue = x_18;
  let x_22 = srcValue.x;
  srcValue.x = (x_22 + bitcast<u32>(1i));
  let x_27 = srcValue;
  textureStore(Dst, vec2i(), x_27);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
