SKIP: FAILED


struct Inner {
  @size(64)
  m : mat4x4<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat4x4<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec4<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:62:5 note: %23 declared here
    %23:u32 = mul %50, 4u
    ^^^^^^^

:45:24 error: binary: %23 is not in scope
    %32:u32 = add %31, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:62:5 note: %23 declared here
    %23:u32 = mul %50, 4u
    ^^^^^^^

:50:24 error: binary: %23 is not in scope
    %38:u32 = add %37, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:62:5 note: %23 declared here
    %23:u32 = mul %50, 4u
    ^^^^^^^

:55:24 error: binary: %23 is not in scope
    %44:u32 = add %43, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:62:5 note: %23 declared here
    %23:u32 = mul %50, 4u
    ^^^^^^^

:62:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %23:u32 = mul %50, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(16) {
  m:mat4x4<f32> @offset(0)
}

Outer = struct @align(16) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 16u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:array<Inner, 4> = call %28, %10
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:u32 = add %30, %16
    %32:u32 = add %31, %23
    %33:Inner = call %34, %32
    %l_a_i_a_i:Inner = let %33
    %36:u32 = add %10, %13
    %37:u32 = add %36, %16
    %38:u32 = add %37, %23
    %39:mat4x4<f32> = call %40, %38
    %l_a_i_a_i_m:mat4x4<f32> = let %39
    %42:u32 = add %10, %13
    %43:u32 = add %42, %16
    %44:u32 = add %43, %23
    %45:u32 = div %44, 16u
    %46:ptr<uniform, vec4<u32>, read> = access %a, %45
    %47:vec4<u32> = load %46
    %48:vec4<f32> = bitcast %47
    %l_a_i_a_i_m_i:vec4<f32> = let %48
    %50:i32 = call %i
    %23:u32 = mul %50, 4u
    %51:u32 = add %10, %13
    %52:u32 = add %51, %16
    %53:u32 = add %52, %23
    %54:u32 = div %53, 16u
    %55:ptr<uniform, vec4<u32>, read> = access %a, %54
    %56:u32 = mod %53, 16u
    %57:u32 = div %56, 4u
    %58:u32 = load_vector_element %55, %57
    %59:f32 = bitcast %58
    %l_a_i_a_i_m_i_i:f32 = let %59
    ret
  }
}
%28 = func(%start_byte_offset:u32):array<Inner, 4> {
  $B4: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x4<f32>(vec4<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %64:bool = gte %idx, 4u
        if %64 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %65:u32 = mul %idx, 64u
        %66:u32 = add %start_byte_offset, %65
        %67:ptr<function, Inner, read_write> = access %a_1, %idx
        %68:Inner = call %34, %66
        store %67, %68
        continue  # -> $B7
      }
      $B7: {  # continuing
        %69:u32 = add %idx, 1u
        next_iteration %69  # -> $B6
      }
    }
    %70:array<Inner, 4> = load %a_1
    ret %70
  }
}
%34 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %72:mat4x4<f32> = call %40, %start_byte_offset_1
    %73:Inner = construct %72
    ret %73
  }
}
%40 = func(%start_byte_offset_2:u32):mat4x4<f32> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %75:u32 = div %start_byte_offset_2, 16u
    %76:ptr<uniform, vec4<u32>, read> = access %a, %75
    %77:vec4<u32> = load %76
    %78:vec4<f32> = bitcast %77
    %79:u32 = add 16u, %start_byte_offset_2
    %80:u32 = div %79, 16u
    %81:ptr<uniform, vec4<u32>, read> = access %a, %80
    %82:vec4<u32> = load %81
    %83:vec4<f32> = bitcast %82
    %84:u32 = add 32u, %start_byte_offset_2
    %85:u32 = div %84, 16u
    %86:ptr<uniform, vec4<u32>, read> = access %a, %85
    %87:vec4<u32> = load %86
    %88:vec4<f32> = bitcast %87
    %89:u32 = add 48u, %start_byte_offset_2
    %90:u32 = div %89, 16u
    %91:ptr<uniform, vec4<u32>, read> = access %a, %90
    %92:vec4<u32> = load %91
    %93:vec4<f32> = bitcast %92
    %94:mat4x4<f32> = construct %78, %83, %88, %93
    ret %94
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %96:array<Inner, 4> = call %28, %start_byte_offset_3
    %97:Outer = construct %96
    ret %97
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x4<f32>(vec4<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %101:bool = gte %idx_1, 4u
        if %101 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %102:u32 = mul %idx_1, 256u
        %103:u32 = add %start_byte_offset_4, %102
        %104:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %105:Outer = call %25, %103
        store %104, %105
        continue  # -> $B15
      }
      $B15: {  # continuing
        %106:u32 = add %idx_1, 1u
        next_iteration %106  # -> $B14
      }
    }
    %107:array<Outer, 4> = load %a_2
    ret %107
  }
}

