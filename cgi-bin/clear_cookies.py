#!/usr/bin/env python3
# -*- coding: utf-8 -*-

def main():
    # Clear all cookies by setting expiration in the past
    print("Content-Type: text/html")
    print("Set-Cookie: visit_count=0; Path=/; Max-Age=0")
    print("Set-Cookie: session_id=; Path=/; Max-Age=0")
    print("Set-Cookie: last_visit=; Path=/; Max-Age=0")
    print("Set-Cookie: username=; Path=/; Max-Age=0")
    print()
    
    print("""<!DOCTYPE html>
<html>
<head>
    <title>Cookies Cleared</title>
    <meta http-equiv='refresh' content='2;url=/cgi-bin/session.py'>
    <style>
        body {
            font-family: Arial;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .message {
            background: white;
            padding: 40px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            text-align: center;
        }
        h1 { color: #dc3545; }
    </style>
</head>
<body>
    <div class='message'>
        <h1> Cookies Cleared!</h1>
        <p>All session data has been removed.</p>
        <p>Redirecting back...</p>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
