#!/usr/bin/env python3
"""Checks the Android build of faf-uid, for .github/workflows/android-check.yaml.

usage:
  android_check.py run <exe> <session> <android id> <out dir>
      runs <exe> on the one connected emulator the way an app does: as
      lib/<abi>/libfafuid.so, with FAF_UID_ANDROID_ID set. Saves what it
      prints to android.out and the device data it reads to android-device.json.
  android_check.py check <private key> <session> <android id> <out dir>
      decrypts android.out and linux.out with the job's throwaway key, checks
      every Android field against android-device.json and the structure
      against the Linux build's, and writes both as android.json and linux.json.
  android_check.py decrypt <private key> <output>
      prints what faf-uid sent, as JSON
"""
import base64
import json
import os
import posixpath
import re
import subprocess
import sys
import tempfile

PROPERTIES = ["ro.product.model", "ro.product.manufacturer", "ro.product.brand",
              "ro.product.board", "ro.soc.model", "ro.build.version.release"]


def fail(msg):
    print("FAIL:", msg)
    sys.exit(1)


def adb(*args):
    done = subprocess.run(["adb", *args], capture_output=True)
    if done.returncode != 0:
        fail(f"adb {' '.join(args)} exited with {done.returncode}: "
             f"{done.stderr.decode(errors='replace').strip()}")
    return done.stdout.decode()


def run(exe, session, android_id, out):
    os.makedirs(out, exist_ok=True)
    abi = adb("shell", "getprop", "ro.product.cpu.abi").strip()
    lib = f"/data/local/tmp/lib/{abi}/libfafuid.so"
    adb("shell", "mkdir", "-p", posixpath.dirname(lib))
    adb("push", exe, lib)
    adb("shell", "chmod", "755", lib)
    printed = adb("shell", f"FAF_UID_ANDROID_ID={android_id} {lib} {session}")
    with open(os.path.join(out, "android.out"), "w") as f:
        f.write(printed)
    device = {p: adb("shell", "getprop", p).rstrip("\r\n") for p in PROPERTIES}
    device["/proc/cpuinfo"] = adb("shell", "cat", "/proc/cpuinfo")
    with open(os.path.join(out, "android-device.json"), "w") as f:
        json.dump(device, f, indent=2)
    print(f"ran {lib} on the emulator ({abi})")


def decrypt(key, path):
    """The inverse of uid.cpp's output, for keys made for testing."""
    with open(path, "rb") as f:
        raw = f.read().strip()
    if not raw:
        fail(f"{path}: empty output")
    outer = base64.b64decode(raw, validate=True)
    pad, inner = outer[0], outer[1:].decode("ascii")
    if len(inner) <= 24 + 344:
        fail(f"{path}: too short")
    iv = base64.b64decode(inner[:24], validate=True)
    rsa = base64.b64decode(inner[-344:], validate=True)
    body = base64.b64decode(inner[24:-344], validate=True)
    if len(iv) != 16 or len(rsa) != 256 or not body or len(body) % 16:
        fail(f"{path}: bad layout")
    with tempfile.TemporaryDirectory() as tmp:
        rsa_path = os.path.join(tmp, "rsa")
        with open(rsa_path, "wb") as f:
            f.write(rsa)
        pkeyutl = ["openssl", "pkeyutl", "-decrypt", "-inkey", key, "-in", rsa_path,
                   "-pkeyopt", "rsa_padding_mode:pkcs1"]
        # OpenSSL 3.2 and later answer a wrong key with random bytes unless
        # implicit rejection is off; older ones do not know the option.
        done = subprocess.run(pkeyutl + ["-pkeyopt", "rsa_pkcs1_implicit_rejection:0"],
                              capture_output=True)
        if done.returncode != 0 and b"rsa_pkcs1_implicit_rejection" in done.stderr:
            done = subprocess.run(pkeyutl, capture_output=True)
        if done.returncode != 0:
            fail(f"{path}: openssl cannot decrypt the AES key with {key}")
        aes_key = done.stdout
    plain = subprocess.run(
        ["openssl", "enc", "-d", "-aes-128-cbc", "-K", aes_key.hex(), "-iv", iv.hex()],
        input=body, capture_output=True, check=True).stdout
    if len(aes_key) != 16 or pad != 16 - len(plain) % 16:
        fail(f"{path}: bad key or pad count")
    text = plain.decode("utf-8")
    if not text.startswith("2"):
        fail(f"{path}: no '2' format prefix")
    return json.loads(text[1:])


def flatten(d, prefix=""):
    for k, v in sorted(d.items()):
        if isinstance(v, dict):
            yield from flatten(v, prefix + k + ".")
        else:
            yield prefix + k, v


def expected_android(device, android_id, names):
    """What machine_info_android.cpp reads, field by field."""
    def prop(name):
        return device[name] or "invalid"

    processor = prop("ro.soc.model")
    if processor == "invalid":
        cpuinfo = device["/proc/cpuinfo"]
        m = (re.search(r"model name\s*: (.*)", cpuinfo)
             or re.search(r"Hardware\s*: (.*)", cpuinfo))
        processor = m.group(1) if m else "invalid"
    want = {n: "invalid" for n in names}
    want.update({
        "uuid": android_id or "invalid",
        "model": prop("ro.product.model"),
        "manufacturer": prop("ro.product.manufacturer"),
        "motherboard.vendor": prop("ro.product.brand"),
        "motherboard.name": prop("ro.product.board"),
        "processor.name": processor,
        "os.type": "Android",
        "os.version": prop("ro.build.version.release"),
    })
    return want


def check(key, session, android_id, out):
    android = decrypt(key, os.path.join(out, "android.out"))
    linux = decrypt(key, os.path.join(out, "linux.out"))
    for name, data in (("android", android), ("linux", linux)):
        if data.get("session") != session:
            fail(f"{name}: session is not {session}")
        with open(os.path.join(out, f"{name}.json"), "w") as f:
            json.dump(data, f, indent=2, sort_keys=True)
            f.write("\n")
    a, l = dict(flatten(android)), dict(flatten(linux))
    if a.keys() != l.keys():
        fail(f"structure differs from the Linux build's: only Android {sorted(a.keys() - l.keys())}, "
             f"only Linux {sorted(l.keys() - a.keys())}")
    types = [n for n in a if type(a[n]) is not type(l[n])]
    if types:
        fail(f"value types differ from the Linux build's: {types}")
    print(f"structure: the same {len(a)} fields and value types as the Linux build")

    with open(os.path.join(out, "android-device.json")) as f:
        device = json.load(f)
    machine = dict(flatten(android["machine"]))
    want = expected_android(device, android_id, machine.keys())
    wrong = [n for n in sorted(machine) if machine[n] != want[n]]
    for n in wrong:
        print(f"  {n}: got {machine[n]!r}, the device has {want[n]!r}")
    if wrong:
        fail(f"{len(wrong)} Android field(s) differ from the device")
    print(f"android: session {session}, uuid from FAF_UID_ANDROID_ID, and all "
          f"{len(machine)} machine fields as the device reports them")


if len(sys.argv) == 6 and sys.argv[1] == "run":
    run(*sys.argv[2:])
elif len(sys.argv) == 6 and sys.argv[1] == "check":
    check(*sys.argv[2:])
elif len(sys.argv) == 4 and sys.argv[1] == "decrypt":
    print(json.dumps(decrypt(*sys.argv[2:]), indent=2, sort_keys=True))
else:
    fail(__doc__)
