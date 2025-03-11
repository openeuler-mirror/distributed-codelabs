#!/usr/bin/python
# health_check.py

import urllib.request as urllib2
import time

RETRY_TIME = 5
STATUS_OK = 200


def check(url):
    for x in range(RETRY_TIME):
        try:
            response = urllib2.urlopen(url)
            if response.status == STATUS_OK:
                return True
            else:
                time.sleep(1)
        except Exception:
            time.sleep(1)
    return False
