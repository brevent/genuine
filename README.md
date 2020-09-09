# What's `genuine`?

A module anti Xposed hook, anti fake signature, anti virtual app (binder proxy), and optional anti odex, anti overlay.

Since 2019-03, Genuine switch to pure c for hide itself. If you want to hide your package name and / or class name, contact me, or do it like `fill_XXX` in `genuine.c`.

- Xposed hook: hook any Java method in [Xposed](https://github.com/rovo89/XposedBridge).
  - [EdXposed](https://github.com/ElderDrivers/EdXposed)
  - (optional) [TaiChi](https://github.com/taichi-framework/TaiChi), closed source

- fake signature: fake your signature.
`genuine` module requires usage of [Apk Sign v2](https://source.android.com/security/apksigning/v2) or [Apk Sign v3](https://source.android.com/security/apksigning/v3).

- (optional) PLT Hook: currently only check `jniRegisterNativeMethods` by flag `CHECK_HOOK`.

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
// #define CHECK_HOOK

/* define to turn off xposed check */
// #define NO_CHECK_XPOSED

/* define to turn on xposed-epic check
 */
// #define CHECK_XPOSED_EPIC

/* check use arm32 on arm64-v8a */
// #define CHECK_ARM64

/* genuine false handler */
// #define GENUINE_FALSE_CRASH
// #define GENUINE_FALSE_NATIVE

/* genuine fake handler */
// #define GENUINE_FAKE_CRASH
#define GENUINE_FAKE_NATIVE

/* genuine overlay handler */
// #define GENUINE_OVERLAY_CRASH
// #define GENUINE_OVERLAY_NATIVE

/* genuine odex handler */
// #define GENUINE_ODEX_CRASH
// #define GENUINE_ODEX_NATIVE

/* genuine dex handler */
// #define GENUINE_DEX_CRASH
// #define GENUINE_DEX_NATIVE

/* genuine proxy handler */
// #define GENUINE_PROXY_CRASH
// #define GENUINE_PROXY_NATIVE

/* genuine error handler */
// #define GENUINE_ERROR_CRASH
#define GENUINE_ERROR_NATIVE

/* genuine fatal handler */
// #define GENUINE_FATAL_CRASH
#define GENUINE_FATAL_NATIVE

/* genuine noapk handler */
// #define GENUINE_NOAPK_CRASH
#define GENUINE_NOAPK_NATIVE

```

# practices

1. make sure libgenuine.so always loaded

2. crash or show native activity for fake, error, fatal

# And license?

[CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/).

For commercial usage, contact me. However, if you are individial, or enterprise less than 5 staff, you can use it under [CC BY-ND 4.0](https://creativecommons.org/licenses/by-nd/4.0/).
