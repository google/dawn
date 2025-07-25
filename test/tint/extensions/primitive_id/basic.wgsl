enable chromium_experimental_primitive_id;

@fragment
fn main(@builtin(primitive_id) prim_id : u32) -> @location(0) vec4f {
    return vec4f(f32(prim_id));
}
