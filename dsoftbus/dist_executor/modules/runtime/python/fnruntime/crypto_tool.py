#!/usr/bin/env python3
# coding=UTF-8
# Copyright (c) 2022 Huawei Technologies Co., Ltd
#
# This software is licensed under Mulan PSL v2.
# You can use this software according to the terms and
# conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

""" crypto tool decrypts redis pwd"""

import hashlib
import hmac
import json
import string
from dataclasses import dataclass

from Cryptodome.Cipher import AES

from fnruntime import log

_NONCE_SIZE = 12
_TAG_SIZE = 16
_BYTE_SIZE = 32
_DEFAULT_SLICE_LEN = 1024
_CIPHER_TEXT_LENGTH = 2
_ITER_KEY_FACTORY_ITER = 10000
# resource key
_APPLE_TXT = "apple"
_BOY_TXT = "boy"
_CAT_TXT = "cat"
_DOG_TXT = "dog"
_WDO_JSON = "wdo"


@dataclass
class RootKeyFactor:
    """
    RootKeyFactor
    """
    iter_count: int
    component3: str
    component3byte: bytes
    k1_data: bytes = None
    k2_data: bytes = None
    mac_data: bytes = None
    salt_data: bytes = None


@dataclass
class RootKey:
    """
    RootKey
    """
    root_key: bytes
    mac_secret_key: bytes


def _resource_checker(resource) -> bool:
    if not isinstance(resource, dict):
        return False
    if _APPLE_TXT not in resource:
        return False
    if _BOY_TXT not in resource:
        return False
    if _CAT_TXT not in resource:
        return False
    if _WDO_JSON not in resource:
        return False
    resource = None
    return True


def _load_root_key(resource: dict) -> RootKey:
    # data3 used to create root key
    data3 = "0B6AA66FADD74F59F019109582E1AAED1EEEEA14CEDFAFCA6DB384D8C3360" \
            "D5E34087FD513B16929A2567E5E184AE2B49A71B9E25E6371C91227D8CE11" \
            "4957D3D383EBC4899DBA7C43F6D80273E57F60B8FC918C2474CA687F1C5DB" \
            "D7A71B1DC0A1EA455C7F2304A4846FD05FFD9FDD96B606546C51241A190EF" \
            "8B70382ABE55"

    f = RootKeyFactor(iter_count=_ITER_KEY_FACTORY_ITER,
                      component3=data3,
                      component3byte=str.encode(data3))
    if _resource_checker(resource) is False:
        log.get_logger().error("failed to check resource")
        raise ValueError("failed to check resource")

    f.k1_data = bytes.fromhex(resource[_APPLE_TXT])
    f.k2_data = bytes.fromhex(resource[_BOY_TXT])
    f.mac_data = bytes.fromhex(resource[_CAT_TXT])
    f.salt_data = bytes.fromhex(resource[_DOG_TXT])
    root_key = _encrypt_pbkdf2_sha256(f)
    resource = None
    return root_key


def _hmac_hash(data: bytes, key: bytes) -> str:
    return hmac.new(key, data, 'sha256').hexdigest()


def get_worker_key(resource) -> str:
    """
    get worker key from resource
    """
    root_key = _load_root_key(resource)
    if root_key is None:
        resource = None
        return ""
    work_keys = json.loads(resource[_WDO_JSON])
    if work_keys is None:
        resource = None
        return ""
    if "key" in work_keys and "mac" in work_keys:
        resource = None
        key = work_keys["key"]
        mac = work_keys["mac"]
        worker_key_decrypt = decrypt(key, root_key.root_key.hex())
        work_key_mac = _hmac_hash(str.encode(worker_key_decrypt),
                                  root_key.mac_secret_key)
        if work_key_mac == mac:
            return worker_key_decrypt
        log.get_logger().error("mac has changed")
    resource = None
    return ""


def decrypt(ciphertext: str, key: str) -> str:
    """decrypt cipher text"""
    cipher_texts = ciphertext.split(':')
    if len(cipher_texts) != _CIPHER_TEXT_LENGTH:
        raise ValueError("failed to decrypt: wrong cipher length")
    salt_str = cipher_texts[0]
    encrypt_str = cipher_texts[1]
    salt = bytes.fromhex(salt_str)
    encrypt = bytes.fromhex(encrypt_str)
    secret_bytes = str.encode(key)
    if all(c in string.hexdigits for c in key):
        secret_bytes = bytes.fromhex(key)

    cipher_bytes = _decrypt_gcm_data(encrypt, secret_bytes, salt)
    return cipher_bytes.decode('utf-8')


def _decrypt_gcm_data(encrypt: bytes, secret: bytes, salt: bytes) -> bytes:
    if len(encrypt) <= _TAG_SIZE:
        log.get_logger().error('The length of cipherText is not long enough')
        return b''
    tag = encrypt[-_TAG_SIZE:]
    aesgcm = AES.new(secret, AES.MODE_GCM, nonce=salt)
    try:
        decrypt_byte = aesgcm.decrypt_and_verify(encrypt[:-_TAG_SIZE], tag)
    except Exception as err:
        log.get_logger().error('decryption failed: %s', err)
        return b''
    return decrypt_byte


def _encrypt_pbkdf2_sha256(f: RootKeyFactor) -> RootKey:
    min_length = min(float(len(f.k1_data)), min(
        float(len(f.k2_data)), float(len(f.component3byte))))

    byte_psd = bytes([f.k1_data[i] ^ f.k2_data[i] ^ f.component3byte[i]
                      for i in range(int(min_length))])

    root_key_byte = hashlib.pbkdf2_hmac("sha256", byte_psd, f.salt_data, f.iter_count, _BYTE_SIZE)

    slice_length = len(root_key_byte)
    if slice_length <= 0 or slice_length > _DEFAULT_SLICE_LEN:
        slice_length = _DEFAULT_SLICE_LEN

    mac_secret_key_byte = hashlib.pbkdf2_hmac("sha256", b'\0' * slice_length, f.mac_data,
                                              f.iter_count,
                                              _BYTE_SIZE)
    return RootKey(root_key_byte, mac_secret_key_byte)
