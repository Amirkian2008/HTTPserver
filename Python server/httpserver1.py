# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""
import http.server
import socketserver
import json

PORT = 5000

class MyHandler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        
        try:
            data = json.loads(post_data)
            temperature = data.get('temperature')
            time = data.get('time')
            print(f"Received temperature: {temperature} C, Time: {time}")
            
            response = {
                "status": "success",
                "data": data
            }
            self.send_response(200)
        except json.JSONDecodeError:
            response = {
                "status": "failure",
                "reason": "Invalid JSON"
            }
            self.send_response(400)
        
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode('utf-8'))

with socketserver.TCPServer(("", PORT), MyHandler) as httpd:
    print(f"Serving on port {PORT}")
    httpd.serve_forever()
