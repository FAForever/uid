# FA Forever unique id implementation

This executable collects RSA encrypted user information.

It is used in the [FA Forever](http://www.faforever.com/) [client](https://github.com/FAForever/client) ([or this one](https://github.com/FAForever/downlords-faf-client)) to prevent [smurfing](http://www.urbandictionary.com/define.php?term=smurfing).

[![Travis](https://travis-ci.org/FAForever/uid.svg?branch=master)](https://travis-ci.org/FAForever/uid)


## Key generation instructions

Create a 2048 bit RSA key using openssl:
```
openssl genrsa -out faf_priv.pem 2048
```

Create the public key part which is stored encrypted in this repo:
```
openssl rsa -in faf_priv.pem -pubout -out faf_pub.pem
```

Now generate the UID_PUBKEY_BYTES string variable you need to pass to CMake with
```
./encode_openssl_modulus.py `openssl rsa -noout -inform PEM -in faf_pub.pem -pubin -modulus`
```
which should result in
```
200,172,159,159,117,211,197,121,7,80,96,139,82,253,240,67,219,77,244,65,25,119,3,147,242,142,113,95,120,226,30,104,158,211,48,73,96,59,85,198,183,199,146,127,140,87,183,110,75,173,39,218,35,146,181,21,115,29,238,23,204,109,15,252,99,204,103,130,138,78,12,7,65,20,247,29,195,136,20,53,200,221,58,114,11,170,65,151,100,61,139,170,244,158,7,192,99,91,142,217,139,253,106,198,180,112,173,49,106,90,121,163,2,24,206,176,198,187,35,37,111,218,197,202,247,139,30,126,152,38,34,73,25,199,10,194,12,196,144,195,98,48,149,14,219,39,182,154,73,246,96,81,152,95,163,251,8,117,35,226,61,16,164,190,128,239,187,122,78,102,209,233,11,126,80,71,187,78,239,28,48,175,91,51,100,83,165,203,222,119,117,138,82,131,199,90,134,250,62,51,231,180,158,11,109,138,75,37,221,145,184,14,177,203,192,191,48,25,159,137,191,252,5,174,209,207,247,198,32,56,152,65,134,251,180,147,36,250,95,50,253,103,240,3,100,211,86,117,5,63,205,61,176,76,48,209
```
for the provided example keyfile `faf_priv_example.pem`.

The private key must be supplied to the server as base64 encoded DER encoded environment variable FAF_PRIVATE_KEY you can generate with
```
openssl rsa -inform PEM -in ./faf_priv.pem -outform DER | openssl enc -base64
```

## Build instructions
You need:
* [CMake](https://cmake.org/)
* [Crypto++](https://www.cryptopp.com/)
* [JsonCpp](https://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html)
* a recent C++ compiler
* [UID_PUBKEY_BYTES bytes string](## Key generation instructions)

Example build commandline script:
```bash
mkdir build
cd build
cmake \
  -DJSONCPP_LIBRARIES=/path/to/libjsoncpp.a \
  -DJSONCPP_INCLUDE_DIRS=/path/to/jsoncpp/include \
  -DCRYPTOPP_LIBRARIES=/path/to/libcryptopp.a \
  -DCRYPTOPP_INCLUDE_DIRS=/path/to/croptopp/include \
  -DUID_PUBKEY_BYTES=200,172,... \
  ..
make
```

## Signing issues

### MacOS

Since the file is downloaded from the internet, MacOS will block the execution. We need to set the execution bit and remove a quarantine tag locally on the users machine.

```sh
chmod +x ./faf-uid-macos
xattr -d com.apple.quarantine ./faf-uid-macos
```

## Android

`machine_info_android.cpp` is used when CMake runs with the NDK's toolchain file:
```bash
cmake \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-26 \
  -DJSONCPP_LIBRARIES=/path/to/libjsoncpp.a \
  -DJSONCPP_INCLUDE_DIRS=/path/to/jsoncpp/include \
  -DCRYPTOPP_LIBRARIES=/path/to/libcryptopp.a \
  -DCRYPTOPP_INCLUDE_DIRS=/path/to/cryptopp/include \
  -DUID_PUBKEY_BYTES=200,172,... \
  ..
```
Crypto++ builds for Android with its `GNUmakefile-cross` after `source TestScripts/setenv-android.sh 26 arm64-v8a` in its folder, with `ANDROID_NDK_ROOT` and `ANDROID_SDK_ROOT` set. The `build-android` job in `.github/workflows/release.yaml` has every step.

### Running it from an app

Since Android 10 an app may execute its own files only from its `nativeLibraryDir`. So the app ships the executable in its APK as `lib/<abi>/libfafuid.so`, with native libraries extracted on install (`android:extractNativeLibs="true"`, or `useLegacyPackaging = true` in Gradle's `packaging.jniLibs`), and runs it like `faf-uid <session>` on the other platforms:

```java
ProcessBuilder pb = new ProcessBuilder(
    new File(getApplicationInfo().nativeLibraryDir, "libfafuid.so").getPath(), session);
pb.environment().put("FAF_UID_ANDROID_ID",
    Settings.Secure.getString(getContentResolver(), Settings.Secure.ANDROID_ID));
```

Only Java can read `Settings.Secure.ANDROID_ID`, so the app passes it in `FAF_UID_ANDROID_ID`. Without it, `uuid` is `invalid`.

### What it collects

| field | value |
|---|---|
| `uuid` | `Settings.Secure.ANDROID_ID`, from `FAF_UID_ANDROID_ID` |
| `model`, `manufacturer` | `Build.MODEL`, `Build.MANUFACTURER` |
| `motherboard.vendor`, `motherboard.name` | `Build.BRAND`, `Build.BOARD` |
| `processor.name` | `Build.SOC_MODEL` (Android 12 and later), else the `model name` or `Hardware` line of `/proc/cpuinfo` |
| `os.type`, `os.version` | `Android`, `Build.VERSION.RELEASE` |

faf-uid reads the `Build` values itself, from the system properties behind them (`ro.product.model` and so on). Every other field is `invalid`:
* Android gives apps no hardware serials. Since Android 10 the serial number and the IMEI need a permission that only system apps get, and the MAC address is hidden.
* faf-uid uses neither the MediaDrm (Widevine) device id, which the user cannot reset, nor the Advertising ID.
* The display size only Java knows, and it is the same for every phone of a model. The bootloader version changes with every system update.

`ANDROID_ID` (Android 8.0 and later) is scoped to the app's signing key and the user. It stays the same through app updates and reinstalls. Another signing key, another user or work profile, or a factory reset gives a new value.
