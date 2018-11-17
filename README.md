# What's `genuine`?

A module anti Xposed hook, anti fake signature, anti virtual app (binder proxy), and optional anti odex, anti overlay.

- Xposed hook: hook any Java method in [Xposed](https://github.com/rovo89/XposedBridge).

- fake signature: fake your signature.
`genuine` module requires usage of [Apk Sign v2](https://source.android.com/security/apksigning/v2).

- virtual app (binder proxy): run your app in virtual app, like [VirtualApp](https://github.com/asLody/VirtualApp).

- (optional) odex: modify odex codes without modify apk, like [URET](https://www.uret.in/)

- (optional) overlay: overlay resources, prevent from loading apk from `/data`.

# How to use?

1. run `java SimpleApkV2 <package-name> <apk>`, save output to `src/main/jni/genuine.h`

2. modify `build.gradle` for `rootProject.XXX`

3. search `FIXME` in `src/main/jni/genuine.cpp`

4. define your own methods in `src/main/jni/genuine_extra.cpp`
   > don't forget to update your own class

# And license?

[CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/).

For Commercial usage, contact me.
