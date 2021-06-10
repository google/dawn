bug/tint/757.wgsl:3:5 warning: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
  [[offset(0)]] level : i32;
    ^^^^^^

bug/tint/757.wgsl:10:5 warning: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
  [[offset(0)]] values : [[stride(4)]] array<f32>;
    ^^^^^^

[[block]]
struct Constants {
  level : i32;
};

[[group(0), binding(0)]] var<uniform> constants : Constants;

[[group(0), binding(1)]] var myTexture : texture_2d_array<f32>;

[[block]]
struct Result {
  values : [[stride(4)]] array<f32>;
};

[[group(0), binding(3)]] var<storage, read_write> result : Result;

[[stage(compute)]]
fn main([[builtin(global_invocation_id)]] GlobalInvocationID : vec3<u32>) {
  var flatIndex : u32 = ((((2u * 2u) * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  var texel : vec4<f32> = textureLoad(myTexture, vec2<i32>(GlobalInvocationID.xy), 0, 0);
  {
    var i : u32 = 0u;
    loop {
      if (!((i < 1u))) {
        break;
      }
      result.values[(flatIndex + i)] = texel.r;

      continuing {
        i = (i + 1u);
      }
    }
  }
}
