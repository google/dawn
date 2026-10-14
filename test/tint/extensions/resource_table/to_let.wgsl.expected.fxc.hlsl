SKIP: FAILED

%fs = @fragment func():vec4<f32> [@location(0)] {
  $B1: {
    %2:texture_2d<f32> = getResource<texture_2d<f32>> 2u
    %3:sampler = getResource<sampler> 3u
    %4:vec4<f32> = textureSample %2, %3, vec2<f32>(0.0f)
    ret %4
  }
}
Failed to generate: resource tables not supported by the HLSL backend for compiling with FXC

tint executable returned error: exit status 1
