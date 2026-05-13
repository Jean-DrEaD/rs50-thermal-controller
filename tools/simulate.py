#!/usr/bin/env python3
"""Simulador local do dashboard RS50 Thermal."""
import re
import http.server
import socketserver
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DASHBOARD_H = ROOT / 'src' / 'rs50_thermal' / 'dashboard.h'
PORT = 8080

def load_dashboard_html() -> str:
    if not DASHBOARD_H.exists():
        return f"<h1>dashboard.h não encontrado em {DASHBOARD_H}</h1>"
    raw = DASHBOARD_H.read_text(encoding='utf-8')
    match = re.search(r'R"rawliteral\((.*?)\)rawliteral"', raw, re.DOTALL)
    if not match:
        return "<h1>HTML não encontrado dentro de dashboard.h</h1>"
    return match.group(1)

class DashboardHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path in ('/', '/index.html'):
            html = load_dashboard_html()
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(html.encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()
    def log_message(self, format, *args):
        print(f"[simulate] {format % args}")

if __name__ == '__main__':
    print(f"[simulate] Servindo dashboard de: {DASHBOARD_H}")
    print(f"[simulate] Acesse: http://localhost:{PORT}")
    with socketserver.TCPServer(("", PORT), DashboardHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[simulate] Encerrando servidor.")
