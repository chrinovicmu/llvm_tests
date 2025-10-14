; ModuleID = 'Solution Module'
source_filename = "Solution Module"

declare i32 @baz()

declare void @bar(i32)

define void @foo(i32 %0, i32 %1) {
bb:
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  store i32 %1, i32* %3, align 4
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* %3, align 4
  %7 = add i32 %5, %6
  store i32 %7, i32* %4, align 4
  %8 = load i32, i32* %4, align 4
  %9 = icmp eq i32 %8, 255
  br i1 %9, label %bb9, label %bb12

bb9:                                              ; preds = %bb
  %10 = load i32, i32* %4, align 4
  %callbar = call void @bar(i32 %10)
  %callbaz = call i32 @baz()
  store i32 %callbaz, i32* %4, align 4
  br label %bb12

bb12:                                             ; preds = %bb9, %bb
  %11 = load i32, i32* %4, align 4
  %bb12_callbar = call void @bar(i32 %11)
  ret void
}
