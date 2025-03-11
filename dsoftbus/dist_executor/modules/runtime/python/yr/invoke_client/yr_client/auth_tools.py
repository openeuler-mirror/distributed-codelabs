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

"""
Authorization tool
"""

import hmac
import base64
import hashlib
from urllib import parse

__APP_KEY = \
    "a9abff86a849f0d40f5a399252c05def4d744a28d3ad27fd73c80db11b706ac8"


def _sign(sign_key: bytes, sign_struct: dict):
    if "query_str" in sign_struct:
        canonical_query_str = _canonical_query(sign_struct["query_str"])
    else:
        canonical_query_str = ""
    if "body" in sign_struct:
        body_str = sign_struct["body"]
    else:
        body_str = ""
    if isinstance(body_str, str):
        body_str = body_str.encode()
    sign_bytes = sign_struct["method"].encode() + b'&' + \
                 sign_struct["path"].encode() + b'&' + \
                 canonical_query_str.encode() + b'&' + \
                 body_str + b'&' + \
                 (("appid=" + sign_struct["appid"]).encode()) + b'&' + \
                 ("timestamp=" + sign_struct["timestamp"]).encode()
    hash_hmac = hmac.new(sign_key, sign_bytes, hashlib.sha256)
    signkey = "\"" + base64.b64encode(hash_hmac.digest()).decode() + "\""
    return signkey


def get_authorization(sign_key: bytes, sign_struct):
    """
    get authorization for frontend
    """
    appid = sign_struct["appid"]
    cur_time = sign_struct["timestamp"]
    signkey = _sign(sign_key, sign_struct)
    authorization = "CLOUDSOA-HMAC-SHA256" + " appid=" + appid + \
                    ", timestamp=" + cur_time + ", signature=" + signkey
    return authorization


def _canonical_query(query):
    pairs = query.split("&")
    pairs.sort()
    pairs = _escape_query_pairs(pairs)
    return '#'.join(pairs)


def _escape_query_pairs(pairs):
    escaped = []
    for pair in pairs:
        kv = pair.split("=")
        if len(kv) > 1:
            k = _url_encode(kv[0])
            v = _url_encode(kv[1])
            escaped.append("".join([k, "=", v]))
    return escaped


def _url_encode(url):
    if url is None or url == "":
        return ""
    encode_url = parse.quote(url)

    encode_url.replace("+", "%20")
    encode_url.replace("*", "%2A")
    encode_url.replace("%7E", "~")
    return encode_url


def _canonical_header(headers):
    kvs = []
    for k, v in headers.items():
        k = _url_encode(k)
        v = _url_encode(v)
        kvs.append("".join([k, "=", v]))
    kvs.sort()
    return "&".join(kvs)
