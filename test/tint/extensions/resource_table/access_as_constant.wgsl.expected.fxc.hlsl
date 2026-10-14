SKIP: FAILED

%fs = @fragment func():void {
  $B1: {
    %2:texture_1d<f32> = getResource<texture_1d<f32>> 2u
    %3:vec4<f32> = textureLoad %2, 0i, 0i
    %texture_load:vec4<f32> = let %3
    ret
  }
}
Failed to generate: resource tables not supported by the HLSL backend for compiling with FXC

tint executable returned error: exit status 1
