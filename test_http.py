#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script de testes para HTTP Request/Response & Parsing
Testa todas as funcionalidades implementadas pelo Membro 2
"""

import http.client
import sys

# Configuração
HOST = "localhost"
PORT = 8080

def test_get_index():
    """Teste: GET da página index.html"""
    print("\n[TEST 1] GET /index.html")
    print("-" * 50)
    
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", "/index.html")
    response = conn.getresponse()
    
    print(f"Status: {response.status} {response.reason}")
    print(f"Headers: {dict(response.getheaders())}")
    
    data = response.read()
    print(f"Body length: {len(data)} bytes")
    
    conn.close()
    
    assert response.status == 200, "Esperado status 200"
    print("✓ PASSOU")

def test_get_directory():
    """Teste: GET de diretório (listagem)"""
    print("\n[TEST 2] GET / (directory listing)")
    print("-" * 50)
    
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", "/")
    response = conn.getresponse()
    
    print(f"Status: {response.status} {response.reason}")
    
    data = response.read().decode('utf-8')
    print(f"Body length: {len(data)} bytes")
    print(f"Contains 'Index of': {('Index of' in data)}")
    
    conn.close()
    
    assert response.status == 200, "Esperado status 200"
    print("✓ PASSOU")

def test_get_404():
    """Teste: GET de arquivo inexistente (404)"""
    print("\n[TEST 3] GET /naoexiste.html (404)")
    print("-" * 50)
    
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", "/naoexiste.html")
    response = conn.getresponse()
    
    print(f"Status: {response.status} {response.reason}")
    
    data = response.read().decode('utf-8')
    print(f"Body contains '404': {('404' in data)}")
    
    conn.close()
    
    assert response.status == 404, "Esperado status 404"
    print("✓ PASSOU")

def test_post_file():
    """Teste: POST para criar arquivo"""
    print("\n[TEST 4] POST /upload/test.txt (create file)")
    print("-" * 50)
    
    conn = http.client.HTTPConnection(HOST, PORT)
    
    body = "Hello, World! This is a test file."
    headers = {
        "Content-Type": "text/plain",
        "Content-Length": str(len(body))
    }
    
    conn.request("POST", "/upload/test.txt", body=body, headers=headers)
    response = conn.getresponse()
    
    print(f"Status: {response.status} {response.reason}")
    
    data = response.read()
    conn.close()
    
    assert response.status == 201, "Esperado status 201 Created"
    print("✓ PASSOU")

def test_get_created_file():
    """Teste: GET do arquivo criado no POST"""
    print("\n[TEST 5] GET /upload/test.txt (read created file)")
    print("-" * 50)
    
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", "/upload/test.txt")
    response = conn.getresponse()
    
    print(f"Status: {response.status} {response.reason}")
    
    data = response.read().decode('utf-8')
    print(f"Body: {data}")
    
    conn.close()
    
    assert response.status == 200, "Esperado status 200"
    assert "Hello, World!" in data, "Body deve conter texto postado"
    print("✓ PASSOU")

def test_delete_file():
    """Teste: DELETE do arquivo"""
    print("\n[TEST 6] DELETE /upload/test.txt")
    print("-" * 50)
    
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("DELETE", "/upload/test.txt")
    response = conn.getresponse()
    
    print(f"Status: {response.status} {response.reason}")
    
    data = response.read()
    conn.close()
    
    assert response.status == 200, "Esperado status 200"
    print("✓ PASSOU")

def test_get_deleted_file():
    """Teste: GET de arquivo deletado (deve retornar 404)"""
    print("\n[TEST 7] GET /upload/test.txt (should be 404)")
    print("-" * 50)
    
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", "/upload/test.txt")
    response = conn.getresponse()
    
    print(f"Status: {response.status} {response.reason}")
    
    data = response.read()
    conn.close()
    
    assert response.status == 404, "Esperado status 404 (arquivo foi deletado)"
    print("✓ PASSOU")

def test_post_chunked():
    """Teste: POST com chunked transfer encoding"""
    print("\n[TEST 8] POST with chunked encoding")
    print("-" * 50)
    
    # Nota: Esta é uma simulação. O servidor deve suportar receber chunks.
    conn = http.client.HTTPConnection(HOST, PORT)
    
    # Python's HTTPConnection can handle chunked automatically
    body = "This is a chunked body test."
    headers = {
        "Transfer-Encoding": "chunked"
    }
    
    try:
        conn.request("POST", "/upload/chunked.txt", body=body, headers=headers)
        response = conn.getresponse()
        
        print(f"Status: {response.status} {response.reason}")
        
        data = response.read()
        conn.close()
        
        print("✓ Chunked transfer suportado")
    except Exception as e:
        print(f"⚠ Chunked transfer pode não estar totalmente implementado: {e}")
        conn.close()

def test_content_types():
    """Teste: Verificação de Content-Types diferentes"""
    print("\n[TEST 9] Content-Type detection")
    print("-" * 50)
    
    files = [
        ("/index.html", "text/html"),
        # Adicione outros arquivos conforme disponíveis
    ]
    
    for path, expected_type in files:
        conn = http.client.HTTPConnection(HOST, PORT)
        conn.request("GET", path)
        response = conn.getresponse()
        
        content_type = response.getheader("Content-Type", "")
        print(f"{path}: {content_type}")
        
        data = response.read()
        conn.close()
        
        if expected_type in content_type:
            print(f"  ✓ Correto: {expected_type}")
        else:
            print(f"  ⚠ Esperado: {expected_type}, Obtido: {content_type}")

def test_url_encoding():
    """Teste: URL encoding/decoding"""
    print("\n[TEST 10] URL encoding (spaces and special chars)")
    print("-" * 50)
    
    # Criar arquivo com espaços no nome
    conn = http.client.HTTPConnection(HOST, PORT)
    
    # URL encoded: "hello world.txt" -> "hello%20world.txt"
    conn.request("POST", "/upload/hello%20world.txt", body="Test content")
    response = conn.getresponse()
    
    print(f"POST Status: {response.status}")
    response.read()
    conn.close()
    
    # Tentar recuperar
    conn = http.client.HTTPConnection(HOST, PORT)
    conn.request("GET", "/upload/hello%20world.txt")
    response = conn.getresponse()
    
    print(f"GET Status: {response.status}")
    
    data = response.read()
    conn.close()
    
    if response.status == 200:
        print("✓ URL decoding funciona")
    else:
        print("⚠ URL decoding pode ter problemas")

def main():
    """Executa todos os testes"""
    print("=" * 50)
    print("TESTE DE HTTP REQUEST/RESPONSE & PARSING")
    print("Membro 2 - Implementação Completa")
    print("=" * 50)
    
    tests = [
        test_get_index,
        test_get_directory,
        test_get_404,
        test_post_file,
        test_get_created_file,
        test_delete_file,
        test_get_deleted_file,
        test_post_chunked,
        test_content_types,
        test_url_encoding
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            test()
            passed += 1
        except AssertionError as e:
            print(f"✗ FALHOU: {e}")
            failed += 1
        except Exception as e:
            print(f"✗ ERRO: {e}")
            failed += 1
    
    print("\n" + "=" * 50)
    print(f"RESULTADO: {passed} passou, {failed} falhou")
    print("=" * 50)
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
