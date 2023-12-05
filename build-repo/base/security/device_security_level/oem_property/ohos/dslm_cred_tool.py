#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2022 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import ast
import base64
import errno
import json
import os
import sys
import shutil
import subprocess
import tempfile

from datetime import datetime

CRED_VERSION = '1.0.1'
ATTESTATION_KEY_USERPUBLICKEY = 'userPublicKey'
ATTESTATION_KEY_SIGNATURE = 'signature'


def _message(message, file):
    if message and file:
        file.write(str(message) + os.linesep)


def _error_message(message, file=sys.stderr):
    _message(message, file)


def _info_message(message, file=sys.stdout):
    _message(message, file)


def run_command(command, input_data=None):
    """
    input:
        command: str tuple/list of command and options to compose full cmd
        input_data: bytes or None that will be redirect to command
    output:
        recode: int, non-zero means failure
        out: bytes or None, stdout info
        err: bytes or None, stderr info
    """
    TIMEOUT_SECONDS = 30
    try:
        proc = subprocess.Popen(
            command,
            stdin=None if input_data is None else subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
    except FileNotFoundError:
        return errno.ENOENT, b'', b'executable not found'

    try:
        if input_data is None:
            proc.wait(timeout=TIMEOUT_SECONDS)
            out = proc.stdout.read()
            err = proc.stderr.read()
        else:
            out, err = proc.communicate(
                input=input_data, timeout=TIMEOUT_SECONDS)
    except subprocess.TimeoutExpired:
        proc.kill()
        return errno.ETIMEDOUT, b'', b'timeout'

    return proc.returncode, out, err


class OpenSslWrapperException(Exception):
    """raised when openssl execution failed"""
    pass


class OpenSslWrapper:
    """wrapper for openssl:"""

    PEM_PERFIX = '-----BEGIN PUBLIC KEY-----'
    PEM_SUFFIX = '-----END PUBLIC KEY-----'

    def __init__(self, store_dir: str = 'artifacts'):
        self.exe = shutil.which('openssl')
        if self.exe is None or not os.path.exists(self.exe):
            raise OpenSslWrapperException('openssl binary not found')
        store_dir = os.path.join(os.getcwd(), store_dir)
        if not os.path.exists(store_dir):
            os.makedirs(store_dir, 0o700)
        self.store_dir = store_dir

    def generate_ecc_key_pair(self, key_alias: str, curve: str = 'brainpoolP384r1') -> bool:
        sk_file = os.path.join(self.store_dir, '{}.key.pem'.format(key_alias))
        pk_file = os.path.join(self.store_dir, '{}.pub.pem'.format(key_alias))

        try:
            if not os.path.isfile(sk_file):
                code, res, err = run_command([self.exe, 'ecparam', '-genkey', '-name', curve, '-out', sk_file])
                if code != 0:
                    raise OpenSslWrapperException('ecparam failed, err: {}'.format(err.decode('utf8')))

            if not os.path.isfile(pk_file):
                code, res, err = run_command(
                    [self.exe, 'ec', '-in', sk_file, '-pubout', '-out', pk_file])
                if code != 0:
                    raise OpenSslWrapperException(
                        'ec failed, err: {}'.format(err.decode('utf8')))
        except OpenSslWrapperException:
            if os.path.exists(sk_file):
                os.remove(sk_file)
            if os.path.exists(pk_file):
                os.remove(pk_file)
            return False
        return True

    def get_ecc_public_key(self, key_alias: str) -> bytes:
        sk_file = os.path.join(self.store_dir, '{}.key.pem'.format(key_alias))
        pk_file = os.path.join(self.store_dir, '{}.pub.pem'.format(key_alias))

        try:
            if not os.path.isfile(pk_file):
                code, res, err = run_command([self.exe, 'ec', '-in', sk_file, '-pubout', '-out', pk_file])
                if code != 0:
                    raise OpenSslWrapperException('ec failed, err: {}'.format(err.decode('utf8')))
            with open(pk_file) as fp:
                pem_str = fp.read().replace(OpenSslWrapper.PEM_PERFIX, '')
                pem_str = pem_str.replace(OpenSslWrapper.PEM_SUFFIX, '')
                pem_str = pem_str.replace(os.linesep, '')
                return base64.b64decode(pem_str)

        except OpenSslWrapperException as ex:
            if os.path.exists(sk_file):
                os.remove(sk_file)
            if os.path.exists(pk_file):
                os.remove(pk_file)
            raise ex

    def digest_sign(self, key_alias: str, content: bytes, digest: str = '-sha384') -> bytes:
        sk_file = os.path.join(self.store_dir, '{}.key.pem'.format(key_alias))

        if not os.path.isfile(sk_file):
            raise OpenSslWrapperException('path not exists')

        code, res, err = run_command(
            [self.exe, 'dgst', digest, '-sign', sk_file, ], content
        )
        if code != 0:
            raise OpenSslWrapperException('dgst failed, err: {}'.format(err.decode('utf8')))

        return res

    def digest_verify_with_key_alias(self, key_alias: str, content: bytes, sig: bytes, digest: str = '-sha384') -> bool:
        pk_file = os.path.join(self.store_dir, '{}.pub.pem'.format(key_alias))

        if not os.path.isfile(pk_file):
            raise OpenSslWrapperException('path not exists')
        try:
            with tempfile.NamedTemporaryFile('wb', dir=self.store_dir, delete=False) as signature_file:
                signature_file.write(sig)
                signature_file.close()

            code, res, err = run_command(
                [self.exe, 'dgst', digest, '-verify', pk_file, '-signature', signature_file.name, ], content
            )
        finally:
            os.remove(signature_file.name)
        return True if code == 0 else False

    def digest_verify_with_pub_key(self, pub_key: bytes, content: bytes, sig: bytes, digest: str = '-sha384') -> bool:
        try:
            with tempfile.NamedTemporaryFile('wb+', dir=self.store_dir, delete=False) as pubkey_file:
                # create an temp pubkey pem file
                pub_key_base64 = base64.b64encode(pub_key).decode('utf8')

                pub_key = '{}{}{}{}{}{}'.format(
                    OpenSslWrapper.PEM_PERFIX, os.linesep,
                    pub_key_base64, os.linesep,
                    OpenSslWrapper.PEM_SUFFIX, os.linesep
                )
                pubkey_file.write(pub_key.encode('utf8'))
                pubkey_file.close()

            with tempfile.NamedTemporaryFile('wb', dir=self.store_dir, delete=False) as signature_file:
                # create an temp signature file
                signature_file.write(sig)
                signature_file.close()

            code, res, err = run_command(
                [self.exe, 'dgst', digest, '-verify', pubkey_file.name, '-signature', signature_file.name, ], content
            )
        finally:
            os.remove(pubkey_file.name)
            os.remove(signature_file.name)

        return True if code == 0 else False


class CredInitializationException(Exception):
    """raised when CredInitialization execution failed"""
    pass


class CredInitialization:
    KEY_ALIAS_ROOT = 'root'
    KEY_ALIAS_OEM = 'oem'
    KEY_ALIAS_DEVICE = 'device'

    def __init__(self, store_dir: str):
        self.ssl = OpenSslWrapper(store_dir)

    def process(self):
        ssl = self.ssl
        try:
            success = ssl.generate_ecc_key_pair(CredInitialization.KEY_ALIAS_ROOT)
            if not success:
                raise CredInitializationException('generate_ecc_key_pair root error')
            success = ssl.generate_ecc_key_pair(CredInitialization.KEY_ALIAS_OEM)
            if not success:
                raise CredInitializationException('generate_ecc_key_pair oem error')
            success = ssl.generate_ecc_key_pair(CredInitialization.KEY_ALIAS_DEVICE)
            if not success:
                raise CredInitializationException('generate_ecc_key_pair device error')

        except CredInitializationException:
            _error_message('cred init failed')
            return


class CredCreationException(Exception):
    """raised when CredInitialization execution failed"""
    pass


class CredCreation:
    def __init__(self, store_dir: str, file: str, payload: dict):
        self.ssl = OpenSslWrapper(store_dir)
        self.payload = payload
        self.file = file

    def _gene_head(self):
        head = {"typ": "DSL"}
        return base64.b64encode(json.dumps(head, ensure_ascii=True).encode('utf8')).decode('utf8')

    def _gene_attestation(self, root_pk: str, root_sign: str,
                          oem_pk: str, oem_sign: str,
                          device_pk: str, device_sign: str):
        data = [
            {
                ATTESTATION_KEY_USERPUBLICKEY: device_pk,
                ATTESTATION_KEY_SIGNATURE: device_sign
            },
            {
                ATTESTATION_KEY_USERPUBLICKEY: oem_pk,
                ATTESTATION_KEY_SIGNATURE: oem_sign
            },
            {
                ATTESTATION_KEY_USERPUBLICKEY: root_pk,
                ATTESTATION_KEY_SIGNATURE: root_sign
            },
        ]
        return base64.b64encode(json.dumps(data, ensure_ascii=True).encode('utf8')).decode('utf8')

    def _gene_payload(self):
        # add sign time

        self.payload['signTime'] = datetime.now().strftime('%Y%m%d%H%M%S')
        self.payload['version'] = CRED_VERSION
        return base64.b64encode(json.dumps(self.payload, ensure_ascii=True).encode('utf8')).decode('utf8')

    def process(self):
        ssl = self.ssl
        try:
            # root self signed
            root_pub_bytes = ssl.get_ecc_public_key(CredInitialization.KEY_ALIAS_ROOT)
            root_pub_self_signed_bytes = ssl.digest_sign(CredInitialization.KEY_ALIAS_ROOT, root_pub_bytes)
            root_pub_str = base64.b64encode(root_pub_bytes).decode('utf8')
            root_pub_self_sign_str = base64.b64encode(root_pub_self_signed_bytes).decode('utf8')

            # oem signed by root
            oem_pub_bytes = ssl.get_ecc_public_key(CredInitialization.KEY_ALIAS_OEM)
            oem_pub_signed_bytes = ssl.digest_sign(CredInitialization.KEY_ALIAS_ROOT, oem_pub_bytes)
            oem_pub_signed_str = base64.b64encode(oem_pub_signed_bytes).decode('utf8')
            oem_pub_str = base64.b64encode(oem_pub_bytes).decode('utf8')

            # device signed by oem
            device_pub_bytes = ssl.get_ecc_public_key(CredInitialization.KEY_ALIAS_DEVICE)
            device_pub_signed_bytes = ssl.digest_sign(CredInitialization.KEY_ALIAS_OEM, device_pub_bytes)
            device_pub_signed_str = base64.b64encode(device_pub_signed_bytes).decode('utf8')
            device_pub_str = base64.b64encode(device_pub_bytes).decode('utf8')

            attestation = self._gene_attestation(root_pub_str, root_pub_self_sign_str,
                                                 oem_pub_str, oem_pub_signed_str,
                                                 device_pub_str, device_pub_signed_str)

            head = self._gene_head()
            payload = self._gene_payload()
            head_payload = '{}.{}'.format(head, payload)

            head_payload_signed_bytes = ssl.digest_sign(
                CredInitialization.KEY_ALIAS_DEVICE, head_payload.encode('utf8'))
            head_payload_signed_string = base64.b64encode(head_payload_signed_bytes).decode('utf8')
            cred = '{}.{}.{}'.format(head_payload, head_payload_signed_string, attestation)
        except (CredCreationException, OpenSslWrapperException):
            _error_message('cred create failed, please init first')
            return

        with open(self.file, 'w') as fp:
            fp.write(cred)


class CredVerificationException(Exception):
    """raised when CredVerification execution failed"""
    pass


class CredVerification:
    def __init__(self, store_dir: str, file: str):
        self.ssl = OpenSslWrapper(store_dir)
        self.file = file

    def process(self):
        try:
            head, payload, signature, attestation = self._split_file(self.file)

            self._check_head(head)
            self._check_payload(payload)
            self._check_signature(signature)
            self._check_attestation(attestation, '{}.{}'.format(head, payload), signature)
        except CredVerificationException as ex:
            _error_message(ex)
            return

        _info_message('verify success!')

    def _check_head(self, header: str):
        header_str = self._base64decode(header).decode('utf8')
        header_obj = ast.literal_eval(header_str)
        if header_obj['typ'] != 'DSL':
            raise CredVerificationException('head error')
        _info_message('head:')
        _info_message(json.dumps(header_obj, indent=2))

    def _check_payload(self, payload: str):
        payload_str = self._base64decode(payload).decode('utf8')
        _info_message('payload:')
        _info_message(json.dumps(json.loads(payload_str), indent=2))

    def _check_signature(self, signature: str):
        self._base64decode(signature)

    def _check_attestation(self, attestation, payload, payload_sign):
        ATTES_PARA_LEN = 3
        attes_str = self._base64decode(attestation).decode('utf8')
        attes_obj = json.loads(attes_str)
        if (len(attes_obj) != ATTES_PARA_LEN):
            raise CredVerificationException('attes para error')
        ssl = self.ssl
        device, oem, root = attes_obj

        root_pk_bytes = self._base64decode(root[ATTESTATION_KEY_USERPUBLICKEY])
        root_self_sign_bytes = self._base64decode(root[ATTESTATION_KEY_SIGNATURE])
        verify = ssl.digest_verify_with_pub_key(root_pk_bytes, root_pk_bytes, root_self_sign_bytes, '-sha384')
        if not verify:
            raise CredVerificationException('root_self_sign verify error')

        oem_pk_bytes = self._base64decode(oem[ATTESTATION_KEY_USERPUBLICKEY])
        oem_sign_bytes = self._base64decode(oem[ATTESTATION_KEY_SIGNATURE])
        verify = ssl.digest_verify_with_pub_key(root_pk_bytes, oem_pk_bytes, oem_sign_bytes, '-sha384')
        if not verify:
            raise CredVerificationException('oem_sign verify error')

        device_pk_bytes = self._base64decode(device[ATTESTATION_KEY_USERPUBLICKEY])
        device_sign_bytes = self._base64decode(device[ATTESTATION_KEY_SIGNATURE])
        verify = ssl.digest_verify_with_pub_key(oem_pk_bytes, device_pk_bytes, device_sign_bytes, '-sha384')
        if not verify:
            raise CredVerificationException('device_sign verify error')

        payload_sign_bytes = self._base64decode(payload_sign)
        verify = ssl.digest_verify_with_pub_key(device_pk_bytes, payload.encode('utf8'),
                                                payload_sign_bytes, '-sha384')
        if not verify:
            verify = ssl.digest_verify_with_pub_key(device_pk_bytes, payload.encode('utf8'),
                                                    payload_sign_bytes, '-sha256')
            if not verify:
                raise CredVerificationException('payload verify error')

    def _split_file(self, cred_file_name: str):
        out = self._get_file_content(cred_file_name).split('.')
        CRED_PARA_LEN = 4
        if (len(out) != CRED_PARA_LEN):
            raise CredVerificationException("cred para error")
        return out

    def _get_file_content(self, file_path: str):
        if not os.path.isfile(file_path):
            raise CredVerificationException('file {} is not existed'.format(file_path))

        with open(file_path, 'r') as fp:
            return fp.read().strip()

    def _base64decode(self, content: str):
        return base64.urlsafe_b64decode(content + '=' * (4 - len(content) % 4))


def init_cred(input_args: argparse.Namespace):
    action = CredInitialization(input_args.dir)
    action.process()


def create_cred(input_args):
    payload = {k: v for k, v in input_args.__dict__.items() if k not in ['dir', 'cred', 'process', 'strict'] and v}
    if input_args.strict:
        init_cred(input_args)
    acton = CredCreation(input_args.dir, input_args.cred, payload)
    acton.process()
    if input_args.strict:
        shutil.rmtree(input_args.dir)


def verify_cred(input_args):
    acton = CredVerification(input_args.dir, input_args.cred)
    acton.process()


class CredCommand:
    def _setup_arguments(self, subparsers, config: dict):
        action = config.get('action')
        arguments = config.get('arguments')
        parser = subparsers.add_parser(action.get('name'), help=action.get('help'))
        parser.set_defaults(process=action.get('process'))
        for item, arg in arguments.items():
            parser.add_argument(*arg.get('name'), dest=item,
                                metavar=arg.get('metavar'), help=arg.get('help'),
                                required=arg.get('required'), type=arg.get('type'),
                                choices=arg.get('choices'), default=arg.get('default'))

    def _setup_init_cmd_parses(self, subparsers):
        cmd_args_def = {
            'action': {
                'name': 'init',
                'help': 'initialization tool for device security level credential',
                'process': init_cred
            },
            'arguments': {
                'dir': {
                    'name': ['-d', '--artifacts-dir'],
                    'metavar': 'dir',
                    'type': str,
                    'default': 'artifacts',
                    'help': 'output artifacts dir',
                }
            }
        }
        self._setup_arguments(subparsers, cmd_args_def)

    def _setup_create_cmd_parses(self, subparsers):
        cmd_args_def = {
            'action': {
                'name': 'create',
                'help': 'creation tool for device security level credential',
                'process': create_cred,
            },
            'arguments': {
                'dir': {
                    'name': ['-d', '--artifacts-dir'],
                    'metavar': 'dir',
                    'type': str,
                    'default': 'artifacts',
                    'help': 'input artifacts dir',
                },
                'type': {
                    'name': ['-t', '--field-type'],
                    'type': str,
                    'choices': ['debug', 'release'],
                    'default': 'debug',
                    'help': 'debug or release',
                },
                'manufacture': {
                    'name': ['-M', '--field-manufacture'],
                    'type': str,
                    'help': 'device manufacture info',
                    'required': True
                },
                'brand': {
                    'name': ['-b', '--field-brand'],
                    'type': str,
                    'help': 'device brand info',
                    'required': True
                },
                'model': {
                    'name': ['-m', '--field-model'],
                    'type': str,
                    'help': 'device model info',
                    'required': True
                },
                'udid': {
                    'name': ['-u', '--field-udid'],
                    'type': str,
                    'help': 'device udid info',
                    'required': False
                },
                'sn': {
                    'name': ['-n', '--field-sn'],
                    'type': str,
                    'help': 'device sn info',
                    'required': False
                },
                'softwareVersion': {
                    'name': ['-s', '--field-software-version'],
                    'type': str,
                    'help': 'device software version info',
                    'required': True
                },
                'securityLevel': {
                    'name': ['-l', '--field-security-level'],
                    'type': str,
                    'choices': ['SL1', 'SL2', 'SL3', 'SL4', 'SL5'],
                    'default': 'SL1',
                    'help': 'device security security info',
                },
                'cred': {
                    'name': ['-f', '--cred-file'],
                    'metavar': 'file',
                    'type': str,
                    'help': 'the device security level credential file to output',
                    'required': True
                },
                'strict': {
                    'name': ['--strict'],
                    'metavar': 'strict',
                    'type': bool,
                    'choices': [True, False],
                    'default': False,
                    'help': 'clean up the artifacts after process',
                },
            }
        }
        self._setup_arguments(subparsers, cmd_args_def)

    def _setup_verify_cmd_parses(self, subparsers):
        cmd_args_def = {
            'action': {
                'name': 'verify',
                'help': 'verification tool for device security level credential',
                'process': verify_cred
            },
            'arguments': {
                'cred': {
                    'name': ['-f', '--cred-file'],
                    'metavar': 'file',
                    'type': str,
                    'help': 'the device security level credential file to verify',
                    'required': True
                },
                'dir': {
                    'name': ['-d', '--artifacts-dir'],
                    'metavar': 'dir',
                    'type': str,
                    'default': 'artifacts',
                    'help': 'input artifacts dir',
                }
            }}
        self._setup_arguments(subparsers, cmd_args_def)

    def parse_args(self):
        parser = argparse.ArgumentParser(description='A collection of device security level credential tools')
        subparsers = parser.add_subparsers(required=True, metavar='action')
        self._setup_init_cmd_parses(subparsers)
        self._setup_create_cmd_parses(subparsers)
        self._setup_verify_cmd_parses(subparsers)

        return parser.parse_args()


if __name__ == '__main__':
    cmd = CredCommand()
    args = cmd.parse_args()
    args.process(args)
