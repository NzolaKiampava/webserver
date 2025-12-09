#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import json
from datetime import datetime

def main():
    print("Content-Type: application/json\r\n\r\n", end='')
    
    data = {
        "message": "Hello from CGI!",
        "timestamp": datetime.now().isoformat(),
        "method": os.environ.get('REQUEST_METHOD', 'UNKNOWN'),
        "path": os.environ.get('PATH_INFO', '/'),
        "query_string": os.environ.get('QUERY_STRING', ''),
        "remote_addr": os.environ.get('REMOTE_ADDR', 'UNKNOWN'),
    }
    
    print(json.dumps(data, indent=2))

if __name__ == "__main__":
    main()
