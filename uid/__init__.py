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

import uid.rsakeys

def decodeUniqueId(serialized_uniqueid, session):
    try:
        message = (base64.b64decode(serialized_uniqueid)).decode('utf-8')

        trailing = ord(message[0])

        message = message[1:]

        iv = (base64.b64decode(message[:24]))
        encoded = message[24:-40]
        key = (base64.b64decode(message[-40:]))

        private_key = rsa.PrivateKey(rsakeys.n, 
                                     rsakeys.e,
                                     rsakeys.d,
                                     rsakeys.p,
                                     rsakeys.q)
        AESkey = rsa.decrypt(key, private_key)

        # What the hell is this?
        cipher = AES.new(AESkey, AES.MODE_CBC, iv)
        DecodeAES = lambda c, e: c.decrypt(base64.b64decode(e)).decode('utf-8')
        decoded = DecodeAES(cipher, encoded)[:-trailing]
        if decoded.startswith('2'):
            data = json.loads(decoded[1:])
            
            if str(data["session"]) != str(session) :
                self.sendJSON(dict(command="notice", style="error", text="Your session is corrupted. Try relogging"))
                return None
            UUID = data['machine']['uuid']
            mem_SerialNumber = data['machine']['memory']['serial0']
            DeviceID = data['machine']['disks']['controller_id']
            Manufacturer = data['machine']['bios']['manufacturer']
            Name = data['machine']['processor']['name']
            ProcessorId = data['machine']['processor']['id']
            SMBIOSBIOSVersion = data['machine']['bios']['smbbversion']
            SerialNumber = data['machine']['bios']['serial']
            VolumeSerialNumber = data['machine']['disks']['vserial']
        else:        
            regexp = re.compile(r'[0-9a-zA-Z\\]("")')
            decoded = regexp.sub('"', decoded)
            decoded = decoded.replace("\\", "\\\\")
            regexp = re.compile('[^\x09\x0A\x0D\x20-\x7F]')
            decoded = regexp.sub('', decoded)
            jstring = json.loads(decoded)

            if str(jstring["session"]) != str(session) :
                self.sendJSON(dict(command="notice", style="error", text="Your session is corrupted. Try relogging"))
                return None

            machine = jstring["machine"]

            UUID = str(machine.get('UUID', 0)).encode()
            mem_SerialNumber = str(machine.get('mem_SerialNumber', 0)).encode()
            DeviceID = str(machine.get('DeviceID', 0)).encode()
            Manufacturer = str(machine.get('Manufacturer', 0)).encode()
            Name = str(machine.get('Name', 0)).encode()
            ProcessorId = str(machine.get('ProcessorId', 0)).encode()
            SMBIOSBIOSVersion = str(machine.get('SMBIOSBIOSVersion', 0)).encode()
            SerialNumber = str(machine.get('SerialNumber', 0)).encode()
            VolumeSerialNumber = str(machine.get('VolumeSerialNumber', 0)).encode()
        m = hashlib.md5()
        m.update(UUID + mem_SerialNumber + DeviceID + Manufacturer + Name + ProcessorId + SMBIOSBIOSVersion + SerialNumber + VolumeSerialNumber)

        return m.hexdigest(), (UUID, mem_SerialNumber, DeviceID, Manufacturer, Name, ProcessorId, SMBIOSBIOSVersion, SerialNumber, VolumeSerialNumber)
    except Exception as ex:
        import traceback
        print("Exception in decodeUniqueId:", ex)
        traceback.print_exc()

def encodeUniqueId(session):
    info = {'machine': uid.collect_machine_info(),
            'session': session}
            
    #prefix the JSON string with Magic byte '2' to indicate the new uid data format for the server
    json_string = '2' + json.dumps(info)
    public_key = rsa.PublicKey(rsakeys.n, rsakeys.e)
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
    a = QApplication.instance()
    if not a:
        a = QApplication(sys.argv)
    desktop_size = QApplication.desktop().screenGeometry()
    machine['desktop'] = {'width':desktop_size.width(), 'height':desktop_size.height()}
    
    exc_value = "invalid"
    if WINDOWS:
        # hostname
        machine['hostname'] = platform.node()
        
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
            machine['memory']['serial0'] = wmi_c.Win32_PhysicalMemory()[0].SerialNumber
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
            diskinfo = wmi_c.Win32_Logicaldisk()[0]
            machine['disks']['vserial'] = diskinfo.VolumeSerialNumber
        except:
            machine['disks']['vserial'] = exc_value
        try:
            controllerinfo = wmi_c.Win32_IDEController()[0]
            machine['disks']['controller_id'] = controllerinfo.DeviceID
        except:
            machine['disks']['controller_id'] = exc_value
            

    else:
        # hostname
        try:
            machine['hostname'] = open('/etc/hostname', 'r').read().strip()
        except:
            machine['hostname'] = exc_value

        # motherboard
        machine['motherboard'] = {}
        for board_trait in ['vendor', 'name']:
            try:
                machine['motherboard'][board_trait] = open('/sys/class/dmi/id/board_{}'.format(board_trait), 'r').read().strip()
            except:
                try:
                    machine['motherboard'][board_trait] = open('/sys/class/virtual/dmi/id/board_{}'.format(board_trait), 'r').read().strip()
                except:
                    pass
                    
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
        try:
            machine['memory']['size'] = int(re.findall(r'MemTotal:\s*([0-9]+) kB',open('/proc/meminfo').read())[0]) * 1024
        except:
            machine['memory']['size'] = exc_value

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
        machine['bios']['smbbversion'] = exc_value
        for bios_trait in [('manufacturer','vendor'), 'version', 'date']:
            # bios_trait is 'string' => machine dict key name equals sysfs name suffix
            # bios_trait is 'tuple' => machine dict key name NOT equals sysfs name suffix
            key = bios_trait if hasattr(bios_trait,'lower') else bios_trait[0]
            sysfs_suffix = bios_trait if hasattr(bios_trait,'lower') else bios_trait[1]
            try:
                machine['bios'][key] = open('/sys/class/dmi/id/bios_{}'.format(sysfs_suffix), 'r').read().strip()
            except:
                try:
                    machine['bios'][key] = open('/sys/class/virtual/dmi/id/bios_{}'.format(sysfs_suffix), 'r').read().strip()
                except:
                    pass
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
        # set smbbversion = version since, why not?
        machine['bios']['smbbversion'] = machine['bios']['version']

        # disk helper
        def find_root_uuid_in_devices( devices ):
            for device in devices:
                if 'mountpoint' in device:
                    if device['mountpoint'] == '/':
                        return device['uuid']
                if 'children' in device:
                    output = find_root_uuid_in_devices(device['children'])
                    if output:
                        return output

        # disk
        machine['disks'] = {}
        machine['disks']['vserial'] = exc_value
        machine['disks']['controller_id'] = exc_value
        try:
            lsblk_json = subprocess.check_output(['lsblk', '-f', '--json']).decode()
            lsblk = json.loads(lsblk_json)
            machine['disks']['serial'] = find_root_uuid_in_devices(lsblk['blockdevices'])
        except:
            pass

    return machine
