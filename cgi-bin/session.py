#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import http.cookies
import datetime

def main():
    # Parse existing cookies
    cookie_string = os.environ.get('HTTP_COOKIE', '')
    cookies = http.cookies.SimpleCookie()
    if cookie_string:
        cookies.load(cookie_string)
    
    # Get visit count
    visit_count = 1
    if 'visit_count' in cookies:
        try:
            visit_count = int(cookies['visit_count'].value) + 1
        except:
            visit_count = 1
    
    # Get or generate session ID
    session_id = cookies.get('session_id')
    if session_id:
        session_id = session_id.value
    else:
        import random
        session_id = ''.join(random.choices('0123456789abcdef', k=32))
    
    # Get username from cookie
    username = cookies.get('username')
    username = username.value if username else ''
    
    # Set new cookies
    visit_cookie = http.cookies.SimpleCookie()
    visit_cookie['visit_count'] = str(visit_count)
    visit_cookie['visit_count']['path'] = '/'
    visit_cookie['visit_count']['max-age'] = 86400  # 24 hours
    
    session_cookie = http.cookies.SimpleCookie()
    session_cookie['session_id'] = session_id
    session_cookie['session_id']['path'] = '/'
    
    last_visit_cookie = http.cookies.SimpleCookie()
    last_visit_cookie['last_visit'] = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    last_visit_cookie['last_visit']['path'] = '/'
    last_visit_cookie['last_visit']['max-age'] = 86400
    
    # Output HTTP headers
    print("Content-Type: text/html")
    print(visit_cookie.output())
    print(session_cookie.output())
    print(last_visit_cookie.output())
    print()  # Empty line separates headers from body
    
    # HTML output
    last_visit = cookies.get('last_visit')
    last_visit = last_visit.value if last_visit else 'First visit'
    
    print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Session & Cookies Demo</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }}
        .container {{
            background: white;
            padding: 40px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            max-width: 600px;
            width: 100%;
        }}
        h1 {{
            color: #667eea;
            margin-bottom: 20px;
            text-align: center;
        }}
        .info-box {{
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            border-left: 5px solid #667eea;
        }}
        .info-box h3 {{
            margin-top: 0;
            color: #667eea;
        }}
        .stat {{
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #e0e0e0;
        }}
        .stat:last-child {{
            border-bottom: none;
        }}
        .label {{
            font-weight: bold;
            color: #666;
        }}
        .value {{
            color: #333;
            font-family: monospace;
        }}
        .success {{
            color: #28a745;
            font-weight: bold;
            text-align: center;
            padding: 10px;
            background: #d4edda;
            border-radius: 5px;
            margin-bottom: 20px;
        }}
        form {{
            margin-top: 20px;
        }}
        input[type="text"] {{
            width: 100%;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 16px;
            box-sizing: border-box;
        }}
        button {{
            width: 100%;
            padding: 12px;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
            margin-top: 10px;
        }}
        button:hover {{
            background: #5568d3;
        }}
        .links {{
            text-align: center;
            margin-top: 20px;
        }}
        .links a {{
            color: #667eea;
            text-decoration: none;
            margin: 0 10px;
        }}
    </style>
</head>
<body>
    <div class='container'>
        <h1> Cookies & Session Demo</h1>
        <div class='success'> Session and Cookies System Working!</div>
        
        <div class='info-box'>
            <h3>Session Information</h3>
            <div class='stat'>
                <span class='label'>Session ID:</span>
                <span class='value'>{session_id[:16]}...</span>
            </div>
            <div class='stat'>
                <span class='label'>Visit Count:</span>
                <span class='value'>{visit_count} visit(s)</span>
            </div>
            <div class='stat'>
                <span class='label'>Last Visit:</span>
                <span class='value'>{last_visit}</span>
            </div>
            {f'''<div class='stat'>
                <span class='label'>Username:</span>
                <span class='value'>{username}</span>
            </div>''' if username else ''}
        </div>
        
        <div class='info-box'>
            <h3>Set Username Cookie</h3>
            <form method='GET' action='/cgi-bin/set_cookie.py'>
                <input type='text' name='username' placeholder='Enter your name' required>
                <button type='submit'>Save Username</button>
            </form>
        </div>
        
        <div class='info-box'>
            <h3>How it works</h3>
            <p>This CGI script demonstrates a working cookies and session system:</p>
            <ul>
                <li><strong>Session ID:</strong> Persistent identifier stored in cookie</li>
                <li><strong>Visit Counter:</strong> Tracks number of visits</li>
                <li><strong>Last Visit Time:</strong> Remembers when you last visited</li>
                <li><strong>User Data:</strong> Can store custom information</li>
            </ul>
            <p><strong>Refresh this page</strong> to see the visit counter increment!</p>
        </div>
        
        <div class='links'>
            <a href='/'>← Home</a> | 
            <a href='/cgi-bin/session.py'> Refresh</a> |
            <a href='/cgi-bin/clear_cookies.py'> Clear Cookies</a>
        </div>
    </div>
</body>
</html>""")

if __name__ == '__main__':
    main()
