// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/task_scheduler/post_task_android.h"

#include "base/no_destructor.h"
#include "base/run_loop.h"
#include "base/task/post_task.h"
#include "base/task/task_scheduler/task_scheduler.h"
#include "jni/PostTask_jni.h"
#include "jni/Runnable_jni.h"

namespace base {

// static
void PostTaskAndroid::SignalNativeSchedulerReady() {
  Java_PostTask_onNativeTaskSchedulerReady(
      base::android::AttachCurrentThread());
}

// static
void PostTaskAndroid::SignalNativeSchedulerShutdown() {
  Java_PostTask_onNativeTaskSchedulerShutdown(
      base::android::AttachCurrentThread());
}

namespace {
std::array<uint8_t, TaskTraitsExtensionStorage::kStorageSize> GetExtensionData(
    JNIEnv* env,
    const base::android::JavaParamRef<jbyteArray>& array_object) {
  if (env->IsSameObject(array_object, nullptr))
    return std::array<uint8_t, TaskTraitsExtensionStorage::kStorageSize>();

  jbyteArray array = static_cast<jbyteArray>(array_object);
  DCHECK_EQ(env->GetArrayLength(array),
            static_cast<jsize>(TaskTraitsExtensionStorage::kStorageSize));

  std::array<uint8_t, TaskTraitsExtensionStorage::kStorageSize> result;
  jbyte* src_bytes = env->GetByteArrayElements(array, nullptr);
  memcpy(&result[0], src_bytes, TaskTraitsExtensionStorage::kStorageSize);
  env->ReleaseByteArrayElements(array, src_bytes, JNI_ABORT);
  return result;
}
}  // namespace

// static
TaskTraits PostTaskAndroid::CreateTaskTraits(
    JNIEnv* env,
    jboolean priority_set_explicitly,
    jint priority,
    jboolean may_block,
    jbyte extension_id,
    const base::android::JavaParamRef<jbyteArray>& extension_data) {
  return TaskTraits(priority_set_explicitly,
                    static_cast<TaskPriority>(priority),
                    /* shutdown_behavior_set_explicitly */ false,
                    TaskShutdownBehavior::SKIP_ON_SHUTDOWN, may_block,
                    /* with_base_sync_primitives */ false,
                    TaskTraitsExtensionStorage(
                        extension_id, GetExtensionData(env, extension_data)));
}

void JNI_PostTask_PostTask(
    JNIEnv* env,
    jboolean priority_set_explicitly,
    jint priority,
    jboolean may_block,
    jbyte extension_id,
    const base::android::JavaParamRef<jbyteArray>& extension_data,
    const base::android::JavaParamRef<jobject>& task) {
  // This could be run on any java thread, so we can't cache |env| in the
  // BindOnce because JNIEnv is thread specific.
  PostTaskWithTraits(
      FROM_HERE,
      PostTaskAndroid::CreateTaskTraits(env, priority_set_explicitly, priority,
                                        may_block, extension_id,
                                        extension_data),
      BindOnce(&PostTaskAndroid::RunJavaTask,
               base::android::ScopedJavaGlobalRef<jobject>(task)));
}

// static
void PostTaskAndroid::RunJavaTask(
    base::android::ScopedJavaGlobalRef<jobject> task) {
  // JNIEnv is thread specific, but we don't know which thread we'll be run on
  // so we must look it up.
  JNI_Runnable::Java_Runnable_run(base::android::AttachCurrentThread(), task);
}

}  // namespace base
