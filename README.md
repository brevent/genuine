1. run `java SimpleApkV2 <package-name> <apk>`, save output to `src/main/jni/genuine.h`

2. modify `build.gradle` for `rootProject.XXX`

3. search `FIXME` in `src/main/jni/genuine.cpp`

4. define your own methods in `src/main/jni/genuine_extra.cpp`
   > don't forget to update your own class
