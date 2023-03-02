@group(0) @binding(0) var<storage> tint_symbol : array<u32>;

@compute @workgroup_size(1)
fn tint_symbol_1(@builtin(local_invocation_index) tint_symbol_2 : u32) {
  let tint_symbol_3 = 0;
  let tint_symbol_4 = 0;
  let tint_symbol_5 = 0;
  let tint_symbol_6 = tint_symbol[tint_symbol_2];
}
