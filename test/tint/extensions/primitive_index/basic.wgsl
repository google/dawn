enable primitive_index;

@fragment
fn main(@builtin(primitive_index) prim_idx : u32) -> @location(0) vec4f {
    return vec4f(f32(prim_idx));
}
