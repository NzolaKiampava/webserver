#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import urllib.parse

def main():
    # Parse query string
    query_string = os.environ.get('QUERY_STRING', '')
    params = urllib.parse.parse_qs(query_string)
    username = params.get('username', [''])[0]
    
    if username:
        # Set username cookie
        print("Content-Type: text/html")
        print(f"Set-Cookie: username={urllib.parse.quote(username)}; Path=/; Max-Age=86400")
        print()
        
        print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Cookie Set</title>
    <meta http-equiv='refresh' content='2;url=/cgi-bin/session.py'>
    <style>
        body {{
            font-family: Arial;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }}
        .message {{
            background: white;
            padding: 40px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            text-align: center;
        }}
        h1 {{ color: #28a745; }}
    </style>
</head>
<body>
    <div class='message'>
        <h1> Cookie Set!</h1>
        <p>Username <strong>{username}</strong> has been saved.</p>
        <p>Redirecting back...</p>
    </div>
</body>
</html>""")
    else:
        print("Content-Type: text/html")
        print()
        print("""<!DOCTYPE html>
<html>
<head>
    <title>Error</title>
    <meta http-equiv='refresh' content='2;url=/cgi-bin/session.py'>
</head>
<body>
    <h1>Error: No username provided</h1>
    <p>Redirecting back...</p>
</body>
</html>""")

if __name__ == '__main__':
    main()
