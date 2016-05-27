import platform
if platform.system() == "Windows":
    WINDOWS = True
else:
    WINDOWS = False

import base64
import json
import re
if WINDOWS:
    import wmi
else:
    import subprocess
import sys
import hashlib
import rsa

from Crypto.Cipher import AES
from Crypto import Random

from PyQt4.QtGui import QApplication
qApp = QApplication.instance()
if not qApp:
    qApp = QApplication(sys.argv)

def decodeUniqueId(serialized_uniqueid, session):
    message = (base64.b64decode(serialized_uniqueid)).decode('utf-8')

    trailing = ord(message[0])

    message = message[1:]

    iv = (base64.b64decode(message[:24]))
    encoded = message[24:-40]
    key = (base64.b64decode(message[-40:]))

    # The JSON string is AES encrypted
    # first decrypt the AES key with our rsa private key
    private_key = rsa.PrivateKey(13731707816857396218511477189051880183926672022487649441793167544537,
                                 65537  ,
                                 13257759923579836515652436320509365545753327507215179875192524262973,
                                 559894335379760802227172596053317511,
                                 24525534460968531815139548638367)
    AESkey = rsa.decrypt(key, private_key)

    # now decrypt the message
    cipher = AES.new(AESkey, AES.MODE_CBC, iv)
    DecodeAES = lambda c, e: c.decrypt(base64.b64decode(e)).decode('utf-8')
    decoded = DecodeAES(cipher, encoded)[:-trailing]

    # since the legacy uid.dll generated JSON is flawed,
    # there's a new JSON format, starting with '2' as magic byte
    if decoded.startswith('2'):
        data = json.loads(decoded[1:])
        if str(data["session"]) != str(session) :
            print(dict(command="notice", style="error", text="Your session is corrupted. Try relogging"))
            return None
        UUID = str(data['machine']['uuid']).encode()
        mem_SerialNumber = str(data['machine']['memory']['serial0']).encode()
        DeviceID = str(data['machine']['disks']['controller_id']).encode()
        Manufacturer = str(data['machine']['bios']['manufacturer']).encode()
        Name = str(data['machine']['processor']['name']).encode()
        ProcessorId = str(data['machine']['processor']['id']).encode()
        SMBIOSBIOSVersion = str(data['machine']['bios']['smbbversion']).encode()
        SerialNumber = str(data['machine']['bios']['serial']).encode()
        VolumeSerialNumber = str(data['machine']['disks']['vserial']).encode()
    else:
        # the old JSON format contains unescaped backspaces in the device id
        # of the IDE controller, which now needs to be corrected to get valid JSON
        regexp = re.compile(r'[0-9a-zA-Z\\]("")')
        decoded = regexp.sub('"', decoded)
        decoded = decoded.replace("\\", "\\\\")
        regexp = re.compile('[^\x09\x0A\x0D\x20-\x7F]')
        decoded = regexp.sub('', decoded)
        jstring = json.loads(decoded)

        if str(jstring["session"]) != str(session) :
            #self.sendJSON(dict(command="notice", style="error", text="Your session is corrupted. Try relogging"))
            return None

        machine = jstring["machine"]

        UUID = str(machine.get('UUID', 0)).encode()
        mem_SerialNumber = str(machine.get('mem_SerialNumber', 0)).encode()  # serial number of first memory module
        DeviceID = str(machine.get('DeviceID', 0)).encode() # device id of the IDE controller
        Manufacturer = str(machine.get('Manufacturer', 0)).encode() # BIOS manufacturer
        Name = str(machine.get('Name', 0)).encode() # verbose processor name
        ProcessorId = str(machine.get('ProcessorId', 0)).encode()
        SMBIOSBIOSVersion = str(machine.get('SMBIOSBIOSVersion', 0)).encode()
        SerialNumber = str(machine.get('SerialNumber', 0)).encode() # BIOS serial number
        VolumeSerialNumber = str(machine.get('VolumeSerialNumber', 0)).encode() # https://www.raymond.cc/blog/changing-or-spoofing-hard-disk-hardware-serial-number-and-volume-id/

        for i in machine.values() :
            low = i.lower()
            if "vmware" in low or "virtual" in low or "innotek" in low or "qemu" in low or "parallels" in low or "bochs" in low :
                return "VM"

    m = hashlib.md5()
    m.update(UUID + mem_SerialNumber + DeviceID + Manufacturer + Name + ProcessorId + SMBIOSBIOSVersion + SerialNumber + VolumeSerialNumber)

    return m.hexdigest(), (UUID, mem_SerialNumber, DeviceID, Manufacturer, Name, ProcessorId, SMBIOSBIOSVersion, SerialNumber, VolumeSerialNumber)

def encodeUniqueId(session):
    info = {'machine': collect_machine_info(),
            'session': session}

    #prefix the JSON string with Magic byte '2' to indicate the new uid data format for the server
    json_string = '2' + json.dumps(info)
    public_key = rsa.PublicKey(13731707816857396218511477189051880183926672022487649441793167544537, 65537  )
    iv = Random.get_random_bytes(16)
    ivb64 = base64.b64encode(iv)
    assert (len(ivb64) == 24)

    aeskey = Random.get_random_bytes(16)
    aeskeyencrypted = rsa.encrypt(aeskey, public_key)

    aeskeyencryptedb64 = base64.b64encode(aeskeyencrypted)
    assert (len(aeskeyencryptedb64) == 40)

    aes = AES.new(aeskey, AES.MODE_CBC, iv)

    # insert trailing bytes to make len(json_string) a multiple of 16
    json_string_len = len(json_string)
    trailing = (((json_string_len // 16) + 1) * 16) - json_string_len
    json_string = json_string + "x" * trailing
    trailingbyte = chr(trailing).encode('utf-8')

    encrypted = aes.encrypt(json_string)
    encryptedb64 = base64.b64encode(encrypted)

    msg = trailingbyte + ivb64 + encryptedb64 + aeskeyencryptedb64
    msg = msg.decode('latin-1').encode('utf-8')

    return base64.b64encode(msg)

def collect_machine_info():
    machine = {}
    desktop_size = qApp.desktop().screenGeometry()
    machine['desktop'] = {'width':desktop_size.width(), 'height':desktop_size.height()}

    exc_value = "invalid"
    if WINDOWS:
        # motherboard
        wmi_c = wmi.WMI()
        machine['motherboard'] = {}
        try:
            mbinfo = wmi_c.Win32_BaseBoard()[0]
            machine['motherboard']['vendor'] = mbinfo.Manufacturer
            machine['motherboard']['name'] = mbinfo.Product
        except:
            machine['motherboard']['vendor'] = exc_value
            machine['motherboard']['name'] = exc_value

        # oem computer name
        try:
            sysinfo = wmi_c.Win32_ComputerSystem()[0]
            machine['model'] = sysinfo.Model
            machine['manufacturer'] = sysinfo.Manufacturer
        except:
            machine['model'] = exc_value
            machine['manufacturer'] = exc_value

        # processor
        machine['processor'] = {}
        try:
            procinfo = wmi_c.Win32_Processor()[0]
            machine['processor']['name'] = procinfo.Name
            machine['processor']['id'] = procinfo.ProcessorId if procinfo.ProcessorId else exc_value
        except:
            machine['processor']['name'] = exc_value
            machine['processor']['id'] = exc_value

        # memory
        machine['memory'] = {}
        try:
            if wmi_c.Win32_PhysicalMemory()[0].SerialNumber:
                machine['memory']['serial0'] = wmi_c.Win32_PhysicalMemory()[0].SerialNumber
            else:
                machine['memory']['serial0'] = exc_value
        except:
            machine['memory']['serial0'] = exc_value

        # os uuid
        try:
            prodinfo = wmi_c.Win32_ComputerSystemProduct()[0]
            machine['uuid'] = prodinfo.UUID
        except:
            machine['uuid'] = exc_value

        # os
        machine['os'] = {}
        try:
            osinfo = wmi_c.Win32_OperatingSystem()[0]
            machine['os']['version'] = osinfo.Version
            machine['os']['type'] = 'Windows'
        except:
            machine['os']['version'] = exc_value
            machine['os']['type'] = exc_value

        # bios
        machine['bios'] = {}
        try:
            biosinfo = wmi_c.Win32_BIOS()[0]
            machine['bios']['manufacturer'] = biosinfo.Manufacturer
            machine['bios']['version'] = biosinfo.Version
            machine['bios']['date'] = biosinfo.ReleaseDate
            machine['bios']['serial'] = biosinfo.SerialNumber
            machine['bios']['description'] = biosinfo.Description
            machine['bios']['smbbversion'] = biosinfo.SMBIOSBIOSVersion
        except:
            machine['bios']['manufacturer'] = exc_value
            machine['bios']['version'] = exc_value
            machine['bios']['serial'] = exc_value
            machine['bios']['date'] = exc_value
            machine['bios']['description'] = exc_value
            machine['bios']['smbbversion'] = exc_value

        # disk
        machine['disks'] = {}
        try:
            c_driveinfo = wmi.WMI(moniker='//./root/cimv2:Win32_LogicalDisk.DeviceID="C:"')
            machine['disks']['vserial'] = c_driveinfo.VolumeSerialNumber
        except:
            machine['disks']['vserial'] = exc_value
        try:
            controllerinfo = wmi_c.Win32_IDEController()[0]
            machine['disks']['controller_id'] = controllerinfo.DeviceID
        except:
            machine['disks']['controller_id'] = exc_value

    else:
        # motherboard
        machine['motherboard'] = {}
        for board_trait in ['vendor', 'name']:
            try:
                machine['motherboard'][board_trait] = open('/sys/class/dmi/id/board_{}'.format(board_trait), 'r').read().strip()
            except:
                try:
                    machine['motherboard'][board_trait] = open('/sys/class/virtual/dmi/id/board_{}'.format(board_trait), 'r').read().strip()
                except:
                    machine['motherboard'][board_trait] = exc_value

        # oem computer name
        try:
            machine['manufacturer'] = open('/sys/class/dmi/id/sys_vendor', 'r').read().strip()
        except:
            machine['manufacturer'] = exc_value
        try:
            machine['model'] = open('/sys/class/dmi/id/product_name', 'r').read().strip()
        except:
            machine['model'] = exc_value

        # processor
        machine['processor'] = {}
        machine['processor']['id'] = exc_value
        try:
            machine['processor']['name'] = re.findall(r'model name\s*: (.*)',open('/proc/cpuinfo').read())[0]
        except:
            machine['processor']['name'] = exc_value

        # memory
        machine['memory'] = {}
        machine['memory']['serial0'] = exc_value

        # os uuid
        try:
            machine['uuid'] = open('/var/lib/dbus/machine-id','r').read().strip()
        except:
            machine['uuid'] = exc_value

        # os
        machine['os'] = {}
        try:
            machine['os']['version'] = subprocess.check_output(('uname', '-r')).decode().strip()
            machine['os']['type'] = subprocess.check_output(('uname', '-s')).decode().strip()
        except:
            machine['os']['version'] = exc_value
            machine['os']['type'] = exc_value

        # bios
        machine['bios'] = {}
        machine['bios']['serial'] = exc_value
        machine['bios']['description'] = exc_value
        for bios_trait in [('manufacturer','vendor'), 'version', 'date']:
            # bios_trait is 'string' => machine dict key name equals sysfs name suffix
            # bios_trait is 'tuple' => machine dict key name NOT equals sysfs name suffix
            key = bios_trait if hasattr(bios_trait, 'lower') else bios_trait[0]
            sysfs_suffix = bios_trait if hasattr(bios_trait, 'lower') else bios_trait[1]
            try:
                machine['bios'][key] = open('/sys/class/dmi/id/bios_{}'.format(sysfs_suffix), 'r').read().strip()
            except:
                try:
                    machine['bios'][key] = open('/sys/class/virtual/dmi/id/bios_{}'.format(sysfs_suffix), 'r').read().strip()
                except:
                    machine['bios'][key] = exc_value
        try:
            # checking dmesg only is fallback, because it's a ring buffer
            # and older information are gone if buffer fills up
            dmesg = subprocess.Popen(('dmesg'), stdout=subprocess.PIPE)
            dmi_bootline = subprocess.check_output(('grep', 'DMI:'), stdin=dmesg.stdout)
            dmi_match = re.match(r".*DMI:\s(\w+)\s(\w+)\s*.*BIOS\s([^\n]*)", dmi_bootline.decode())
            if dmi_match:
                if machine['bios']['manufacturer'] == exc_value:
                    machine['bios']['manufacturer'] = dmi_match.group(1)
                # dmi_match.group(2) == motherboard name?
                if machine['bios']['version'] == exc_value:
                    machine['bios']['version'] = dmi_match.group(3)
        except:
            pass
        # set smbbversion = version since, this value is n/a
        machine['bios']['smbbversion'] = machine['bios']['version']

        # disk
        machine['disks'] = {}
        machine['disks']['vserial'] = exc_value
        machine['disks']['controller_id'] = exc_value
        try:
            lspci_output = subprocess.check_output(('lspci', '-n'))
            controller_id_match = re.search(r"..:..\..\s0106:\s(....:....)\s", lspci_output.decode(), re.MULTILINE)
            if controller_id_match:
                machine['disks']['controller_id'] = controller_id_match.group(1)
        except:
            pass

        def is_root_device(device):
            if 'mountpoint' in device and device['mountpoint'] != None:
                return device['mountpoint'] == '/'
            else:
                result = False
                if 'children' in device:
                    for child_dev in device['children']:
                        result |= is_root_device(child_dev)
                return result

        try:
            lsblk_json = subprocess.check_output(['lsblk', '--json', '-o', 'SERIAL,NAME,MOUNTPOINT']).decode()
            lsblk = json.loads(lsblk_json)
            for device in lsblk['blockdevices']:
                if is_root_device(device):
                    machine['disks']['vserial'] = device['serial']
                    if machine['disks']['vserial'] is None:
                        machine['disks']['vserial'] = exc_value
        except:
            pass

    return machine
