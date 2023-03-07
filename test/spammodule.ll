; ModuleID = './test/spammodule.c'
source_filename = "./test/spammodule.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx13.0.0"

%struct.PyModuleDef = type { %struct.PyModuleDef_Base, ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr }
%struct.PyModuleDef_Base = type { %struct._object, ptr, i64, ptr }
%struct._object = type { i64, ptr }
%struct.PyMethodDef = type { ptr, ptr, i32, ptr }

@spammodule = internal global %struct.PyModuleDef { %struct.PyModuleDef_Base { %struct._object { i64 1, ptr null }, ptr null, i64 0, ptr null }, ptr @.str.2, ptr @.str.3, i64 -1, ptr @SpamMethods, ptr null, ptr null, ptr null, ptr null }, align 8
@.str = private unnamed_addr constant [11 x i8] c"spam.error\00", align 1
@Spam_Error = internal global ptr null, align 8
@.str.1 = private unnamed_addr constant [6 x i8] c"error\00", align 1
@.str.2 = private unnamed_addr constant [5 x i8] c"spam\00", align 1
@.str.3 = private unnamed_addr constant [26 x i8] c"Python interface for spam\00", align 1
@SpamMethods = internal global [2 x %struct.PyMethodDef] [%struct.PyMethodDef { ptr @.str.4, ptr @spam_system, i32 1, ptr @.str.5 }, %struct.PyMethodDef zeroinitializer], align 8
@.str.4 = private unnamed_addr constant [7 x i8] c"system\00", align 1
@.str.5 = private unnamed_addr constant [25 x i8] c"Execute a shell command.\00", align 1
@.str.6 = private unnamed_addr constant [2 x i8] c"s\00", align 1
@.str.7 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@.str.8 = private unnamed_addr constant [22 x i8] c"System command failed\00", align 1

; Function Attrs: noinline nounwind optnone ssp uwtable
define ptr @PyInit_spam() #0 {
  %1 = alloca ptr, align 8
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = call ptr @PyModule_Create2(ptr noundef @spammodule, i32 noundef 1013)
  store ptr %4, ptr %2, align 8
  %5 = load ptr, ptr %2, align 8
  %6 = icmp eq ptr %5, null
  br i1 %6, label %7, label %8

7:                                                ; preds = %0
  store ptr null, ptr %1, align 8
  br label %28

8:                                                ; preds = %0
  %9 = call ptr @PyErr_NewException(ptr noundef @.str, ptr noundef null, ptr noundef null)
  store ptr %9, ptr @Spam_Error, align 8
  %10 = load ptr, ptr @Spam_Error, align 8
  call void @_Py_XINCREF(ptr noundef %10)
  %11 = load ptr, ptr %2, align 8
  %12 = load ptr, ptr @Spam_Error, align 8
  %13 = call i32 @PyModule_AddObject(ptr noundef %11, ptr noundef @.str.1, ptr noundef %12)
  %14 = icmp slt i32 %13, 0
  br i1 %14, label %15, label %26

15:                                               ; preds = %8
  %16 = load ptr, ptr @Spam_Error, align 8
  call void @_Py_XDECREF(ptr noundef %16)
  br label %17

17:                                               ; preds = %15
  %18 = load ptr, ptr @Spam_Error, align 8
  store ptr %18, ptr %3, align 8
  %19 = load ptr, ptr %3, align 8
  %20 = icmp ne ptr %19, null
  br i1 %20, label %21, label %23

21:                                               ; preds = %17
  store ptr null, ptr @Spam_Error, align 8
  %22 = load ptr, ptr %3, align 8
  call void @_Py_DECREF(ptr noundef %22)
  br label %23

23:                                               ; preds = %21, %17
  br label %24

24:                                               ; preds = %23
  %25 = load ptr, ptr %2, align 8
  call void @_Py_DECREF(ptr noundef %25)
  store ptr null, ptr %1, align 8
  br label %28

26:                                               ; preds = %8
  %27 = load ptr, ptr %2, align 8
  store ptr %27, ptr %1, align 8
  br label %28

28:                                               ; preds = %26, %24, %7
  %29 = load ptr, ptr %1, align 8
  ret ptr %29
}

declare ptr @PyModule_Create2(ptr noundef, i32 noundef) #1

declare ptr @PyErr_NewException(ptr noundef, ptr noundef, ptr noundef) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal void @_Py_XINCREF(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = icmp ne ptr %3, null
  br i1 %4, label %5, label %7

5:                                                ; preds = %1
  %6 = load ptr, ptr %2, align 8
  call void @_Py_INCREF(ptr noundef %6)
  br label %7

7:                                                ; preds = %5, %1
  ret void
}

declare i32 @PyModule_AddObject(ptr noundef, ptr noundef, ptr noundef) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal void @_Py_XDECREF(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = icmp ne ptr %3, null
  br i1 %4, label %5, label %7

5:                                                ; preds = %1
  %6 = load ptr, ptr %2, align 8
  call void @_Py_DECREF(ptr noundef %6)
  br label %7

7:                                                ; preds = %5, %1
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal void @_Py_DECREF(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %struct._object, ptr %3, i32 0, i32 0
  %5 = load i64, ptr %4, align 8
  %6 = add nsw i64 %5, -1
  store i64 %6, ptr %4, align 8
  %7 = icmp ne i64 %6, 0
  br i1 %7, label %8, label %9

8:                                                ; preds = %1
  br label %11

9:                                                ; preds = %1
  %10 = load ptr, ptr %2, align 8
  call void @_Py_Dealloc(ptr noundef %10)
  br label %11

11:                                               ; preds = %9, %8
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal ptr @spam_system(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %8 = load ptr, ptr %5, align 8
  %9 = call i32 (ptr, ptr, ...) @_PyArg_ParseTuple_SizeT(ptr noundef %8, ptr noundef @.str.6, ptr noundef %6)
  %10 = icmp ne i32 %9, 0
  br i1 %10, label %12, label %11

11:                                               ; preds = %2
  store ptr null, ptr %3, align 8
  br label %25

12:                                               ; preds = %2
  %13 = load ptr, ptr %6, align 8
  %14 = call i32 (ptr, ...) @printf(ptr noundef @.str.7, ptr noundef %13)
  %15 = load ptr, ptr %6, align 8
  %16 = call i32 @"\01_system"(ptr noundef %15)
  store i32 %16, ptr %7, align 4
  %17 = load i32, ptr %7, align 4
  %18 = icmp slt i32 %17, 0
  br i1 %18, label %19, label %21

19:                                               ; preds = %12
  %20 = load ptr, ptr @Spam_Error, align 8
  call void @PyErr_SetString(ptr noundef %20, ptr noundef @.str.8)
  store ptr null, ptr %3, align 8
  br label %25

21:                                               ; preds = %12
  %22 = load i32, ptr %7, align 4
  %23 = sext i32 %22 to i64
  %24 = call ptr @PyLong_FromLong(i64 noundef %23)
  store ptr %24, ptr %3, align 8
  br label %25

25:                                               ; preds = %21, %19, %11
  %26 = load ptr, ptr %3, align 8
  ret ptr %26
}

declare i32 @_PyArg_ParseTuple_SizeT(ptr noundef, ptr noundef, ...) #1

declare i32 @printf(ptr noundef, ...) #1

declare i32 @"\01_system"(ptr noundef) #1

declare void @PyErr_SetString(ptr noundef, ptr noundef) #1

declare ptr @PyLong_FromLong(i64 noundef) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal void @_Py_INCREF(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %struct._object, ptr %3, i32 0, i32 0
  %5 = load i64, ptr %4, align 8
  %6 = add nsw i64 %5, 1
  store i64 %6, ptr %4, align 8
  ret void
}

declare void @_Py_Dealloc(ptr noundef) #1

attributes #0 = { noinline nounwind optnone ssp uwtable "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+crypto,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+v8.5a,+zcm,+zcz" }
attributes #1 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+crypto,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+v8.5a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 1}
!4 = !{!"Homebrew clang version 15.0.7"}
