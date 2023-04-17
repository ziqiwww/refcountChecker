; ModuleID = '/Users/ziqi/Work/pyc/workaround/refcountChecker/test/spammodule.c'
source_filename = "/Users/ziqi/Work/pyc/workaround/refcountChecker/test/spammodule.c"
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
entry:
  %retval = alloca ptr, align 8
  %module = alloca ptr, align 8
  %_py_tmp = alloca ptr, align 8
  %call = call ptr @PyModule_Create2(ptr noundef @spammodule, i32 noundef 1013)
  store ptr %call, ptr %module, align 8
  %0 = load ptr, ptr %module, align 8
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store ptr null, ptr %retval, align 8
  br label %return

if.end:                                           ; preds = %entry
  %call1 = call ptr @PyErr_NewException(ptr noundef @.str, ptr noundef null, ptr noundef null)
  store ptr %call1, ptr @Spam_Error, align 8
  %1 = load ptr, ptr @Spam_Error, align 8
  call void @_Py_XINCREF(ptr noundef %1)
  %2 = load ptr, ptr %module, align 8
  %3 = load ptr, ptr @Spam_Error, align 8
  %call2 = call i32 @PyModule_AddObject(ptr noundef %2, ptr noundef @.str.1, ptr noundef %3)
  %cmp3 = icmp slt i32 %call2, 0
  br i1 %cmp3, label %if.then4, label %if.end8

if.then4:                                         ; preds = %if.end
  %4 = load ptr, ptr @Spam_Error, align 8
  call void @_Py_XDECREF(ptr noundef %4)
  br label %do.body

do.body:                                          ; preds = %if.then4
  %5 = load ptr, ptr @Spam_Error, align 8
  store ptr %5, ptr %_py_tmp, align 8
  %6 = load ptr, ptr %_py_tmp, align 8
  %cmp5 = icmp ne ptr %6, null
  br i1 %cmp5, label %if.then6, label %if.end7

if.then6:                                         ; preds = %do.body
  store ptr null, ptr @Spam_Error, align 8
  %7 = load ptr, ptr %_py_tmp, align 8
  call void @_Py_DECREF(ptr noundef %7)
  br label %if.end7

if.end7:                                          ; preds = %if.then6, %do.body
  br label %do.end

do.end:                                           ; preds = %if.end7
  %8 = load ptr, ptr %module, align 8
  call void @_Py_DECREF(ptr noundef %8)
  store ptr null, ptr %retval, align 8
  br label %return

if.end8:                                          ; preds = %if.end
  %9 = load ptr, ptr %module, align 8
  store ptr %9, ptr %retval, align 8
  br label %return

return:                                           ; preds = %if.end8, %do.end, %if.then
  %10 = load ptr, ptr %retval, align 8
  ret ptr %10
}

declare ptr @PyModule_Create2(ptr noundef, i32 noundef) #1

declare ptr @PyErr_NewException(ptr noundef, ptr noundef, ptr noundef) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal void @_Py_XINCREF(ptr noundef %op) #0 {
entry:
  %op.addr = alloca ptr, align 8
  store ptr %op, ptr %op.addr, align 8
  %0 = load ptr, ptr %op.addr, align 8
  %cmp = icmp ne ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load ptr, ptr %op.addr, align 8
  call void @_Py_INCREF(ptr noundef %1)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare i32 @PyModule_AddObject(ptr noundef, ptr noundef, ptr noundef) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal void @_Py_XDECREF(ptr noundef %op) #0 {
entry:
  %op.addr = alloca ptr, align 8
  store ptr %op, ptr %op.addr, align 8
  %0 = load ptr, ptr %op.addr, align 8
  %cmp = icmp ne ptr %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load ptr, ptr %op.addr, align 8
  call void @_Py_DECREF(ptr noundef %1)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
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

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal ptr @spam_system(ptr noundef %self, ptr noundef %args) #0 {
entry:
  %retval = alloca ptr, align 8
  %self.addr = alloca ptr, align 8
  %args.addr = alloca ptr, align 8
  %command = alloca ptr, align 8
  %sts = alloca i32, align 4
  store ptr %self, ptr %self.addr, align 8
  store ptr %args, ptr %args.addr, align 8
  %0 = load ptr, ptr %args.addr, align 8
  %call = call i32 (ptr, ptr, ...) @_PyArg_ParseTuple_SizeT(ptr noundef %0, ptr noundef @.str.6, ptr noundef %command)
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  store ptr null, ptr %retval, align 8
  br label %return

if.end:                                           ; preds = %entry
  %1 = load ptr, ptr %command, align 8
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.7, ptr noundef %1)
  %2 = load ptr, ptr %command, align 8
  %call2 = call i32 @"\01_system"(ptr noundef %2)
  store i32 %call2, ptr %sts, align 4
  %3 = load i32, ptr %sts, align 4
  %cmp = icmp slt i32 %3, 0
  br i1 %cmp, label %if.then3, label %if.end4

if.then3:                                         ; preds = %if.end
  %4 = load ptr, ptr @Spam_Error, align 8
  call void @PyErr_SetString(ptr noundef %4, ptr noundef @.str.8)
  store ptr null, ptr %retval, align 8
  br label %return

if.end4:                                          ; preds = %if.end
  %5 = load i32, ptr %sts, align 4
  %conv = sext i32 %5 to i64
  %call5 = call ptr @PyLong_FromLong(i64 noundef %conv)
  store ptr %call5, ptr %retval, align 8
  br label %return

return:                                           ; preds = %if.end4, %if.then3, %if.then
  %6 = load ptr, ptr %retval, align 8
  ret ptr %6
}

declare i32 @_PyArg_ParseTuple_SizeT(ptr noundef, ptr noundef, ...) #1

declare i32 @printf(ptr noundef, ...) #1

declare i32 @"\01_system"(ptr noundef) #1

declare void @PyErr_SetString(ptr noundef, ptr noundef) #1

declare ptr @PyLong_FromLong(i64 noundef) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define internal void @_Py_INCREF(ptr noundef %op) #0 {
entry:
  %op.addr = alloca ptr, align 8
  store ptr %op, ptr %op.addr, align 8
  %0 = load ptr, ptr %op.addr, align 8
  %ob_refcnt = getelementptr inbounds %struct._object, ptr %0, i32 0, i32 0
  %1 = load i64, ptr %ob_refcnt, align 8
  %inc = add nsw i64 %1, 1
  store i64 %inc, ptr %ob_refcnt, align 8
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
