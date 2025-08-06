// flags: --glsl-desktop
// [hlsl-dxc] flags: --hlsl-shader-model 62

enable chromium_experimental_barycentric_coord;

@fragment
fn main(@builtin(barycentric_coord) bary_coord : vec3f) -> @location(0) vec4f {
    return vec4f(bary_coord, 1.0);
}
