# Midnight
Midnight is a universal Fortnite Redirect/SSL Bypass that has one main goal, that is to redirect from **Epic Games** backend services to a **Hosted Custom Fortnite Backend** to give ability for Fortnite private servers to have Mobile support.

## Branches
- **Android (Main)**: Android version of the redirect.
- **iOS**: iOS version of the redirect.

## Support
- **5.21 - 18.30:** by hooking **curl_easy_setopt**.
- **18.40+:** by using **Unreal Engine** native functions.

## How to build
Make sure you configured ``includes/opts.h`` correctly to your liking and installed and setup **NDK** & **CMake** from **Android SDK** and run ``Build.bat`` for Windows or ``Build.sh`` for MacOS or Linux

## How to inject
Make sure you have **JDK 21** & **Apktool** jar and  decompile the apk using the following command:
```bash
java -jar apktool.jar decode fortnite.apk -o ./Fortnite
```

Then put the ``libs/arm64-v8a/libmidnight.so`` file inside ``Fortnite/libs/arm64-v8a`` and then go to ``smali/com/epicgames/ue4/GameActivity.smali`` (the smali folder may vary from each version it can be smali_classes3 for example) and find ``onCreate()`` function and do the following

```diff
.method public onCreate(Landroid/os/Bundle;)V
    .locals 11
    
+   const-string v0, "midnight"
+   invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V
```
Optional (disables minimal system requirements for new devices if doing ch1 builds)
```diff
.method private processSystemInfo(Ljava/lang/String;Ljava/lang/String;)Z
    .locals 29
    
+   const/4 v0, 0x1
+   return v0

    :try_start_0 
```

Add this to ``AndroidManifest.xml`` if your phone is running **Android 14+**
```
<uses-sdk android:minSdkVersion="21" android:targetSdkVersion="24" />
```

Now build the apk using
```bash
java -jar apktool.jar build ./Fortnite -o fortnite_modded.apk
```

Note that you still need to sign it for it to work use ``uber-apk-signer`` for example or some other tool this command uses ``uber-apk-signer`` to sign the apk:
```bash
java -jar uber-apk-signer.jar --apks fortnite_modded.apk
```

Now you got a fully working apk you can install on your phone and play **Fortnite mobile** on your **Private Server**. if you found this tool helpful don't forget to star the repo and if you want to use this tool for your **Private Server** don't forget to credit the owner (razerfrfr) it's much appreciated.
