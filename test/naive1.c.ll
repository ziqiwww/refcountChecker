; ModuleID = '/Users/ziqi/Work/pyc/workaround/refcountChecker/test/naive1.c'
source_filename = "/Users/ziqi/Work/pyc/workaround/refcountChecker/test/naive1.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx13.0.0"

%struct._object = type { i64, ptr }

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i64 @sum_list(ptr noundef %list) #0 {
entry:
  %retval = alloca i64, align 8
  %list.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  %n = alloca i32, align 4
  %total = alloca i64, align 8
  %item = alloca ptr, align 8
  store ptr %list, ptr %list.addr, align 8
  store i64 0, ptr %total, align 8
  %0 = load ptr, ptr %list.addr, align 8
  %call = call i64 @PyList_Size(ptr noundef %0)
  %conv = trunc i64 %call to i32
  store i32 %conv, ptr %n, align 4
  %1 = load i32, ptr %n, align 4
  %cmp = icmp slt i32 %1, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i64 -1, ptr %retval, align 8
  br label %return

if.end:                                           ; preds = %entry
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %2 = load i32, ptr %i, align 4
  %3 = load i32, ptr %n, align 4
  %cmp2 = icmp slt i32 %2, %3
  br i1 %cmp2, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %4 = load ptr, ptr %list.addr, align 8
  %5 = load i32, ptr %i, align 4
  %conv4 = sext i32 %5 to i64
  %call5 = call ptr @PyList_GetItem(ptr noundef %4, i64 noundef %conv4)
  store ptr %call5, ptr %item, align 8
  %6 = load ptr, ptr %item, align 8
  %call6 = call i64 @PyLong_AsLong(ptr noundef %6)
  %7 = load i64, ptr %total, align 8
  %add = add nsw i64 %7, %call6
  store i64 %add, ptr %total, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, ptr %i, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  %9 = load ptr, ptr %item, align 8
  call void @_Py_DECREF(ptr noundef %9)
  %10 = load i64, ptr %total, align 8
  store i64 %10, ptr %retval, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  %11 = load i64, ptr %retval, align 8
  ret i64 %11
}

declare i64 @PyList_Size(ptr noundef) #1

declare ptr @PyList_GetItem(ptr noundef, i64 noundef) #1

declare i64 @PyLong_AsLong(ptr noundef) #1

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define internal void @_Py_DECREF(ptr noundef %op) #0 {
entry:
  %op.addr = alloca ptr, align 8
  store ptr %op, ptr %op.addr, align 8
  %0 = load ptr, ptr %op.addr, align 8
  %ob_refcnt = getelementptr inbounds %struct._object, ptr %0, i32 0, i32 0
  %1 = load i64, ptr %ob_refcnt, align 8
  %dec = add nsw i64 %1, -1
  store i64 %dec, ptr %ob_refcnt, align 8
  %cmp = icmp ne i64 %dec, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  %2 = load ptr, ptr %op.addr, align 8
  call void @_Py_Dealloc(ptr noundef %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare void @_Py_Dealloc(ptr noundef) #1

attributes #0 = { noinline nounwind optnone ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+crypto,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+zcm,+zcz" }
attributes #1 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+crypto,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 1}
!4 = !{!"Homebrew clang version 16.0.6"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
