@group(0u) @binding(0u) var Src : texture_storage_2d<r32uint, read>;

@group(0u) @binding(1u) var Dst : texture_storage_2d<r32uint, write>;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var srcValue : vec4<u32>;
  srcValue = textureLoad(Src, vec2<i32>());
  srcValue.x = (srcValue.x + 1u);
  textureStore(Dst, vec2<i32>(), srcValue);
}
