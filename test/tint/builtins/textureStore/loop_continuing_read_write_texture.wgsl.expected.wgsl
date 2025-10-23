@binding(2) @group(0) var tex : texture_storage_2d<r32sint, read_write>;

@compute @workgroup_size(1)
fn foo() {
  for(var i = 0; (i < 3); textureStore(tex, vec2(), vec4())) {
  }
}
