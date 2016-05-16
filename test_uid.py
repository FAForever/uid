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
  
  assert(UUID == info['uuid'])
  assert(mem_SerialNumber == info['memory']['serial0'])
  assert(DeviceID == info['disks']['controller_id'])
  assert(Manufacturer == info['bios']['manufacturer'])
  assert(Name == info['processor']['name'])
  assert(ProcessorId == info['processor']['id'])
  assert(SMBIOSBIOSVersion == info['bios']['smbbversion'])
  assert(SerialNumber == info['bios']['serial'])
  assert(VolumeSerialNumber == info['disks']['vserial'])


if __name__ == "__main__":
  import uid

  print("Using uid from", uid.__file__)
  import json

  info = uid.collect_machine_info()
  print(json.dumps(info, sort_keys=True, indent=2, separators=(',', ': ')))

  #encoded = uid.encodeUniqueId(1234)
  
