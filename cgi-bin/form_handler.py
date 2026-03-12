#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
CGI Script - Form Handler
Processa dados de formulários (GET e POST)
"""

import os
import sys
import urllib.parse
from datetime import datetime

def parse_query_string(qs):
    """Parse query string em dicionário"""
    params = {}
    if qs:
        for param in qs.split('&'):
            if '=' in param:
                key, value = param.split('=', 1)
                params[urllib.parse.unquote(key)] = urllib.parse.unquote(value)
    return params

# Headers
print("Content-Type: text/html\r")
print("\r")

# HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>Form Handler - CGI</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; max-width: 800px; margin: 40px auto; padding: 20px; }")
print("        h1 { color: #2c3e50; }")
print("        .form-container { background: #ecf0f1; padding: 30px; border-radius: 8px; margin: 20px 0; }")
print("        label { display: block; margin: 15px 0 5px; font-weight: bold; }")
print("        input, textarea { width: 100%; padding: 10px; border: 1px solid #bdc3c7; border-radius: 4px; box-sizing: border-box; }")
print("        button { background: #3498db; color: white; padding: 12px 30px; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; margin-top: 15px; }")
print("        button:hover { background: #2980b9; }")
print("        .result { background: #d5f4e6; padding: 20px; border-radius: 8px; border-left: 4px solid #27ae60; margin: 20px 0; }")
print("        .param { background: white; padding: 10px; margin: 5px 0; border-radius: 4px; }")
print("    </style>")
print("</head>")
print("<body>")

print("    <h1> Form Handler (CGI)</h1>")

# Processar dados
request_method = os.environ.get('REQUEST_METHOD', '')
query_string = os.environ.get('QUERY_STRING', '')

if request_method == 'GET' and query_string:
    params = parse_query_string(query_string)
    
    print("    <div class='result'>")
    print("        <h2> Form Submitted (GET)</h2>")
    print(f"        <p><strong>Time:</strong> {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")
    print("        <h3>Parameters Received:</h3>")
    
    for key, value in params.items():
        print(f"        <div class='param'><strong>{key}:</strong> {value}</div>")
    
    print("    </div>")

elif request_method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        params = parse_query_string(post_data)
        
        print("    <div class='result'>")
        print("        <h2> Form Submitted (POST)</h2>")
        print(f"        <p><strong>Time:</strong> {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")
        print("        <h3>Parameters Received:</h3>")
        
        for key, value in params.items():
            print(f"        <div class='param'><strong>{key}:</strong> {value}</div>")
        
        print("    </div>")

# Formulário de teste
print("    <div class='form-container'>")
print("        <h2>Test Form (GET)</h2>")
print("        <form method='GET' action='/cgi-bin/form_handler.py'>")
print("            <label>Name:</label>")
print("            <input type='text' name='name' placeholder='Enter your name'>")
print("            ")
print("            <label>Email:</label>")
print("            <input type='email' name='email' placeholder='your.email@example.com'>")
print("            ")
print("            <label>Message:</label>")
print("            <textarea name='message' rows='4' placeholder='Type your message here...'></textarea>")
print("            ")
print("            <button type='submit'>Submit (GET)</button>")
print("        </form>")
print("    </div>")

print("    <div class='form-container'>")
print("        <h2>Test Form (POST)</h2>")
print("        <form method='POST' action='/cgi-bin/form_handler.py'>")
print("            <label>Username:</label>")
print("            <input type='text' name='username' placeholder='Choose a username'>")
print("            ")
print("            <label>Age:</label>")
print("            <input type='number' name='age' placeholder='Your age'>")
print("            ")
print("            <label>Country:</label>")
print("            <input type='text' name='country' placeholder='Your country'>")
print("            ")
print("            <button type='submit'>Submit (POST)</button>")
print("        </form>")
print("    </div>")

print("</body>")
print("</html>")
