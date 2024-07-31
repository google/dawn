SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> buffer : array<i32, 4>;

fn foo(arg : ptr<storage, array<i32, 4>, read_write>) {
  for(var i = 0; (i < 4); i++) {
    switch(arg[i]) {
      case 1: {
        continue;
      }
      default: {
        arg[i] = 2;
      }
    }
  }
}

@fragment
fn main() {
  foo(&(buffer));
}

Failed to generate: :26:26 error: binary: %10 is not in scope
        %9:u32 = add 0u, %10
                         ^^^

:12:7 note: in block
      $B4: {  # body
      ^^^

:37:13 note: %10 declared here
            %10:u32 = mul %15, 4u
            ^^^^^^^

note: # Disassembly
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        %i:ptr<function, i32, read_write> = var, 0i
        next_iteration  # -> $B4
      }
      $B4: {  # body
        %4:i32 = load %i
        %5:bool = lt %4, 4i
        if %5 [t: $B6, f: $B7] {  # if_1
          $B6: {  # true
            exit_if  # if_1
          }
          $B7: {  # false
            exit_loop  # loop_1
          }
        }
        %6:i32 = load %i
        %7:u32 = convert %6
        %8:u32 = mul %7, 4u
        %9:u32 = add 0u, %10
        %11:u32 = add %9, %8
        %12:u32 = %buffer.Load %11
        %13:i32 = bitcast %12
        switch %13 [c: (1i, $B8), c: (default, $B9)] {  # switch_1
          $B8: {  # case
            continue  # -> $B5
          }
          $B9: {  # case
            %14:i32 = load %i
            %15:u32 = convert %14
            %10:u32 = mul %15, 4u
            %16:u32 = add 0u, %10
            %17:u32 = bitcast 2i
            %18:void = %buffer.Store %16, %17
            exit_switch  # switch_1
          }
        }
        continue  # -> $B5
      }
      $B5: {  # continuing
        %19:i32 = load %i
        %20:i32 = add %19, 1i
        store %i, %20
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
%main = @fragment func():void {
  $B10: {
    %22:void = call %foo
    ret
  }
}

