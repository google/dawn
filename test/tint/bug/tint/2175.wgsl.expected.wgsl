const tint_symbol = vec4();

const tint_symbol_1 = (tint_symbol.x * 2u);

@group(0) @binding(0) var<storage, read_write> tint_symbol_2 : u32;

@compute @workgroup_size(1)
fn tint_symbol_3() {
  tint_symbol_2 = tint_symbol_1;
}
