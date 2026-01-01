from flask import Flask, send_from_directory
import os

app = Flask(__name__, static_folder='static')

@app.route('/')
def index():
    return send_from_directory('static', 'index.html')

@app.route('/<path:filename>')
def serve_static(filename):
    return send_from_directory('static', filename)

if __name__ == '__main__':
    port = 5000
    print(f"""
    ╔══════════════════════════════════════════════════════════╗
    ║                                                          ║
    ║   🪶  Icarus Landing Page Server                         ║
    ║                                                          ║
    ║   Local:    http://localhost:{port}                       ║
    ║   Network:  http://<your-ip>:{port}                       ║
    ║                                                          ║
    ║   To find your IP, run: hostname -I (Linux)              ║
    ║                     or: ipconfig (Windows)               ║
    ║                     or: ifconfig (Mac)                   ║
    ║                                                          ║
    ║   Press Ctrl+C to stop the server                        ║
    ║                                                          ║
    ╚══════════════════════════════════════════════════════════╝
    """)
    
    # Run on 0.0.0.0 to allow access from other devices on the network
    app.run(host='0.0.0.0', port=port, debug=False)