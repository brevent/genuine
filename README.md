# What's `genuine`?

A module anti Xposed hook, anti fake signature, anti virtual app (binder proxy), and optional anti odex, anti overlay.

Since 2019-03, Genuine switch to pure c for hide itself. If you want to hide your package name and / or class name, contact me, or do it like `fill_XXX` in `genuine.c`.

- Xposed hook: hook any Java method in [Xposed](https://github.com/rovo89/XposedBridge).
  - [EdXposed](https://github.com/ElderDrivers/EdXposed)
  - (optional) [TaiChi](https://github.com/taichi-framework/TaiChi), closed source

- fake signature: fake your signature.
`genuine` module requires usage of [Apk Sign v2](https://source.android.com/security/apksigning/v2) or [Apk Sign v3](https://source.android.com/security/apksigning/v3).

- (optional) PLT Hook: currently only check `jniRegisterNativeMethods` by flag `CHECK_JNI_REGISTER_NATIVE_METHODS`.

- virtual app (binder proxy): run your app in virtual app, like [VirtualApp](https://github.com/asLody/VirtualApp).

- (optional) odex: modify odex codes without modify apk, like [URET](https://www.uret.in/)

- (optional) overlay: overlay resources, prevent from loading apk from `/data`.

# How to use?

1. run `java SimpleApkV2 <package-name> <apk>`, save output to `src/main/jni/genuine.h`

2. modify `build.gradle` for `rootProject.XXX`

3. search `FIXME` in `src/main/jni/genuine.c`

4. define your own methods in `src/main/jni/genuine_extra.c`
   > don't forget to update your own class

# features

```c
/* define to turn off maps check */
// #define NO_CHECK_MAPS

#ifndef NO_CHECK_MAPS
/* define to anti odex */
// #define ANTI_ODEX

/* define to anti overlay */
// #define ANTI_OVERLAY
#endif

/* define to check plt hook for jniRegisterNativeMethods */
// #define CHECK_JNI_REGISTER_NATIVE_METHODS

/* define to turn off xposed check */
// #define NO_CHECK_XPOSED

/* define to turn off xposed-edxposed check */
// #define NO_CHECK_XPOSED_EDXPOSED

/* define to turn on xposed-epic check
 * requires stl, refer https://developer.android.com/ndk/guides/cpp-support?hl=en
 */
// #define CHECK_XPOSED_EPIC
```

# practices

1. make sure libgenuine.so always loaded

2. crash for fake signature

# And license?

[CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/).

For commercial usage, contact me. However, if you are individial, or enterprise less than 5 staff, you can use it under [CC BY-ND 4.0](https://creativecommons.org/licenses/by-nd/4.0/).
