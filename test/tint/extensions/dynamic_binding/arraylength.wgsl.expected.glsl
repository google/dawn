SKIP: INVALID

Failed to generate: :8:17 error: length: no matching call to 'length(resource_binding)'

1 candidate function:
 • 'length(ptr<storage, array<T>, A>  ✗ ) -> i32'

    %4:i32 = %3.length
                ^^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %sampled_textures:ptr<handle, resource_binding, read> = var undef @binding_point(0, 0)
}

%fs = @fragment func():void {
  $B2: {
    %3:resource_binding = load %sampled_textures
    %4:i32 = %3.length
    %5:u32 = convert %4
    %t:u32 = let %5
    ret
  }
}


tint executable returned error: exit status 1
