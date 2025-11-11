
; ModuleID = 'test'
source_filename = "test.c"

define i32 @foo(i32 %a, i32 %b) {
entry:
  %0 = add i32 %a, %b
  %1 = call float @llvm.sqrt.f32(float 4.0)
  ret i32 %0
}

declare float @llvm.sqrt.f32(float)
