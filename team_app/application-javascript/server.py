import http.server
import socketserver
import json

# Read the JSON data from the file
with open("data.json", "r") as json_file:
    data = json.load(json_file)

# Define the port you want to use
port = 8080  # You can change this to any available port you prefer

# Create a custom request handler that serves the JSON data
class JSONRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')  # Allow cross-origin requests
        super().end_headers()

# Set up the server
with socketserver.TCPServer(("", port), JSONRequestHandler) as httpd:
    print(f"Serving at port {port}")
    httpd.serve_forever()

