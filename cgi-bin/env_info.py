#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
CGI Script - Environment Info
Mostra todas as variáveis de ambiente CGI e informações da requisição
"""

import os
import sys
from datetime import datetime

# Headers HTTP (obrigatório para CGI)
print("Content-Type: text/html\r")
print("\r")

# HTML output
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>CGI Environment - Python</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }")
print("        .container { background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }")
print("        h1 { color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; }")
print("        h2 { color: #34495e; margin-top: 30px; }")
print("        table { border-collapse: collapse; width: 100%; margin: 20px 0; }")
print("        th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }")
print("        th { background-color: #3498db; color: white; font-weight: bold; }")
print("        tr:nth-child(even) { background-color: #f2f2f2; }")
print("        code { background: #ecf0f1; padding: 2px 6px; border-radius: 3px; font-family: 'Courier New', monospace; }")
print("        .highlight { background: #ffeaa7; padding: 10px; border-left: 4px solid #fdcb6e; margin: 10px 0; }")
print("        pre { background: #2c3e50; color: #ecf0f1; padding: 15px; border-radius: 4px; overflow-x: auto; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")

print("        <h1> CGI Environment Information</h1>")
print(f"        <p class='highlight'><strong>Generated at:</strong> {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")

# Request Info
print("        <h2> Request Information</h2>")
print("        <table>")
print("            <tr><th>Property</th><th>Value</th></tr>")
print(f"            <tr><td><strong>Method</strong></td><td><code>{os.environ.get('REQUEST_METHOD', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Script Name</strong></td><td><code>{os.environ.get('SCRIPT_NAME', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Query String</strong></td><td><code>{os.environ.get('QUERY_STRING', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Content Type</strong></td><td><code>{os.environ.get('CONTENT_TYPE', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Content Length</strong></td><td><code>{os.environ.get('CONTENT_LENGTH', '0')}</code></td></tr>")
print("        </table>")

# Server Info
print("        <h2> Server Information</h2>")
print("        <table>")
print("            <tr><th>Property</th><th>Value</th></tr>")
print(f"            <tr><td><strong>Server Name</strong></td><td><code>{os.environ.get('SERVER_NAME', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Server Port</strong></td><td><code>{os.environ.get('SERVER_PORT', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Server Protocol</strong></td><td><code>{os.environ.get('SERVER_PROTOCOL', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Server Software</strong></td><td><code>{os.environ.get('SERVER_SOFTWARE', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Gateway Interface</strong></td><td><code>{os.environ.get('GATEWAY_INTERFACE', 'N/A')}</code></td></tr>")
print("        </table>")

# Client Info
print("        <h2> Client Information</h2>")
print("        <table>")
print("            <tr><th>Property</th><th>Value</th></tr>")
print(f"            <tr><td><strong>Remote Address</strong></td><td><code>{os.environ.get('REMOTE_ADDR', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>User Agent</strong></td><td><code>{os.environ.get('HTTP_USER_AGENT', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Host</strong></td><td><code>{os.environ.get('HTTP_HOST', 'N/A')}</code></td></tr>")
print(f"            <tr><td><strong>Accept</strong></td><td><code>{os.environ.get('HTTP_ACCEPT', 'N/A')}</code></td></tr>")
print("        </table>")

# All Environment Variables
print("        <h2> All Environment Variables</h2>")
print("        <table>")
print("            <tr><th>Variable</th><th>Value</th></tr>")

for key, value in sorted(os.environ.items()):
    print(f"            <tr><td><code>{key}</code></td><td>{value}</td></tr>")

print("        </table>")

# POST Data
request_method = os.environ.get('REQUEST_METHOD', '')
if request_method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print("        <h2> POST Data</h2>")
        print("        <pre>" + post_data + "</pre>")

# Python Info
print("        <h2> Python Runtime</h2>")
print("        <table>")
print("            <tr><th>Property</th><th>Value</th></tr>")
print(f"            <tr><td><strong>Version</strong></td><td><code>{sys.version}</code></td></tr>")
print(f"            <tr><td><strong>Executable</strong></td><td><code>{sys.executable}</code></td></tr>")
print(f"            <tr><td><strong>Platform</strong></td><td><code>{sys.platform}</code></td></tr>")
print("        </table>")

print("    </div>")
print("</body>")
print("</html>")
