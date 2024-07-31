SKIP: FAILED


struct S {
  a : array<vec4<i32>, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<S>;

var<private> v : u32;

fn idx1() -> i32 {
  v--;
  return 1;
}

fn idx2() -> i32 {
  v--;
  return 2;
}

fn idx3() -> i32 {
  v--;
  return 3;
}

fn idx4() -> i32 {
  v--;
  return 4;
}

fn idx5() -> i32 {
  v--;
  return 0;
}

fn idx6() -> i32 {
  v--;
  return 2;
}

fn main() {
  for(buffer[idx1()].a[idx2()][idx3()]--; (v < 10u); buffer[idx4()].a[idx5()][idx6()]--) {
  }
}

Failed to generate: :73:28 error: binary: %34 is not in scope
        %33:u32 = add %32, %34
                           ^^^

:61:7 note: in block
      $B9: {  # initializer
      ^^^

:79:9 note: %34 declared here
        %34:u32 = mul %39, 4u
        ^^^^^^^

:112:28 error: binary: %59 is not in scope
        %58:u32 = add %57, %59
                           ^^^

:100:7 note: in block
      $B11: {  # continuing
      ^^^^

:118:9 note: %59 declared here
        %59:u32 = mul %64, 4u
        ^^^^^^^

note: # Disassembly
S = struct @align(16) {
  a:array<vec4<i32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
  %v:ptr<private, u32, read_write> = var
}

%idx1 = func():i32 {
  $B2: {
    %4:u32 = load %v
    %5:u32 = sub %4, 1u
    store %v, %5
    ret 1i
  }
}
%idx2 = func():i32 {
  $B3: {
    %7:u32 = load %v
    %8:u32 = sub %7, 1u
    store %v, %8
    ret 2i
  }
}
%idx3 = func():i32 {
  $B4: {
    %10:u32 = load %v
    %11:u32 = sub %10, 1u
    store %v, %11
    ret 3i
  }
}
%idx4 = func():i32 {
  $B5: {
    %13:u32 = load %v
    %14:u32 = sub %13, 1u
    store %v, %14
    ret 4i
  }
}
%idx5 = func():i32 {
  $B6: {
    %16:u32 = load %v
    %17:u32 = sub %16, 1u
    store %v, %17
    ret 0i
  }
}
%idx6 = func():i32 {
  $B7: {
    %19:u32 = load %v
    %20:u32 = sub %19, 1u
    store %v, %20
    ret 2i
  }
}
%main = func():void {
  $B8: {
    loop [i: $B9, b: $B10, c: $B11] {  # loop_1
      $B9: {  # initializer
        %22:i32 = call %idx1
        %23:i32 = call %idx2
        %24:u32 = convert %22
        %25:u32 = mul %24, 64u
        %26:u32 = convert %23
        %27:u32 = mul %26, 16u
        %28:i32 = call %idx3
        %29:u32 = convert %28
        %30:u32 = mul %29, 4u
        %31:u32 = add 0u, %25
        %32:u32 = add %31, %27
        %33:u32 = add %32, %34
        %35:u32 = add %33, %30
        %36:u32 = %buffer.Load %35
        %37:i32 = bitcast %36
        %38:i32 = sub %37, 1i
        %39:u32 = convert %28
        %34:u32 = mul %39, 4u
        %40:u32 = add 0u, %25
        %41:u32 = add %40, %27
        %42:u32 = add %41, %34
        %43:u32 = bitcast %38
        %44:void = %buffer.Store %42, %43
        next_iteration  # -> $B10
      }
      $B10: {  # body
        %45:u32 = load %v
        %46:bool = lt %45, 10u
        if %46 [t: $B12, f: $B13] {  # if_1
          $B12: {  # true
            exit_if  # if_1
          }
          $B13: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B11
      }
      $B11: {  # continuing
        %47:i32 = call %idx4
        %48:i32 = call %idx5
        %49:u32 = convert %47
        %50:u32 = mul %49, 64u
        %51:u32 = convert %48
        %52:u32 = mul %51, 16u
        %53:i32 = call %idx6
        %54:u32 = convert %53
        %55:u32 = mul %54, 4u
        %56:u32 = add 0u, %50
        %57:u32 = add %56, %52
        %58:u32 = add %57, %59
        %60:u32 = add %58, %55
        %61:u32 = %buffer.Load %60
        %62:i32 = bitcast %61
        %63:i32 = sub %62, 1i
        %64:u32 = convert %53
        %59:u32 = mul %64, 4u
        %65:u32 = add 0u, %50
        %66:u32 = add %65, %52
        %67:u32 = add %66, %59
        %68:u32 = bitcast %63
        %69:void = %buffer.Store %67, %68
        next_iteration  # -> $B10
      }
    }
    ret
  }
}
%unused_entry_point = @compute @workgroup_size(1, 1, 1) func():void {
  $B14: {
    ret
  }
}

