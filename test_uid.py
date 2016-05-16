#!/usr/bin/env python2 

def test_info_content():
  """Check for required fields"""
  import uid
  info = uid.collect_machine_info()
  
  assert('uuid' in info)
  assert('serial0' in info['memory'])
  assert('controller_id' in info['disks'])
  assert('manufacturer' in info['bios'])
  assert('name' in info['processor'])
  assert('id' in info['processor'])
  assert('smbbversion' in info['bios'])
  assert('serial' in info['bios'])
  assert('vserial' in info['disks'])

def test_decode():
  """Check JSON encryption, decryption and match of the JSON fields"""
  import uid
  session = 1234
  encoded = uid.encodeUniqueId(session)
  hash, (UUID, mem_SerialNumber, DeviceID, Manufacturer, Name, ProcessorId, SMBIOSBIOSVersion, SerialNumber, VolumeSerialNumber) = uid.decodeUniqueId(encoded, session)
    
  info = uid.collect_machine_info()
  
  assert(UUID == str(info['uuid']).encode())
  assert(mem_SerialNumber == str(info['memory']['serial0']).encode())
  assert(DeviceID == str(info['disks']['controller_id']).encode())
  assert(Manufacturer == str(info['bios']['manufacturer']).encode())
  assert(Name == str(info['processor']['name']).encode())
  assert(ProcessorId == str(info['processor']['id']).encode())
  assert(SMBIOSBIOSVersion == str(info['bios']['smbbversion']).encode())
  assert(SerialNumber == str(info['bios']['serial']).encode())
  assert(VolumeSerialNumber == str(info['disks']['vserial']).encode())


if __name__ == "__main__":
  import uid

  print("Using uid from", uid.__file__)
  import json

  info = uid.collect_machine_info()
  print(json.dumps(info, sort_keys=True, indent=2, separators=(',', ': ')))

  #encoded = uid.encodeUniqueId(1234)
  
