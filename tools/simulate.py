"""
simulate.py - Simulador local do firmware ESP32 Cooler Controller
Replica: leitura NTC, controle térmico, PWM, WebSocket, UDP SimHub
Uso: python simulate.py
Acesse: http://localhost:8080
"""

import asyncio
import json
import math
import random
import socket
import time
from pathlib import Path
from aiohttp import web, WSMsgType

# ===== CONFIG (espelho do config.h) =====
HTTP_PORT       = 8080
UDP_PORT        = 10001
UDP_HOST        = "255.255.255.255"

TEMP_IDLE       = 35.0
TEMP_WARMING    = 45.0
TEMP_WARNING    = 60.0
TEMP_CRITICAL   = 75.0

PWM_MIN         = 80
PWM_MAX         = 255

EMA_ALPHA       = 0.15
TICK_MS         = 200
WS_INTERVAL_MS  = 500
UDP_INTERVAL_MS = 100

# ===== ESTADO GLOBAL =====
state = {
    "temp": 28.0,
    "raw_temp": 28.0,
    "duty": 0,
    "state": "IDLE",
    "relay": False,
    "uptime": 0,
    "hours": 0.0,
}
start_time = time.time()
ws_clients = set()

# ===== SIMULAÇÃO FÍSICA =====
def simulate_physics():
    """Simula aquecimento/resfriamento + ruído NTC."""
    target = 30.0 + 50.0 * (0.5 + 0.5 * math.sin(time.time() / 30.0))
    noise = random.uniform(-0.3, 0.3)
    state["raw_temp"] += (target - state["raw_temp"]) * 0.05 + noise
    # Filtro EMA
    state["temp"] = EMA_ALPHA * state["raw_temp"] + (1 - EMA_ALPHA) * state["temp"]

def update_control():
    """Replica a máquina de estados com histerese."""
    t = state["temp"]
    cur = state["state"]

    if t >= TEMP_CRITICAL:
        new_state = "CRITICAL"
    elif t >= TEMP_WARNING:
        new_state = "WARNING"
    elif t >= TEMP_WARMING:
        new_state = "WARMING"
    elif t < TEMP_IDLE - 2.0:  # histerese
        new_state = "IDLE"
    else:
        new_state = cur

    state["state"] = new_state
    state["relay"] = new_state != "IDLE"

    # Curva linear de duty
    if new_state == "IDLE":
        duty = 0
    elif new_state == "CRITICAL":
        duty = PWM_MAX
    else:
        ratio = (t - TEMP_WARMING) / (TEMP_CRITICAL - TEMP_WARMING)
        ratio = max(0.0, min(1.0, ratio))
        duty = int(PWM_MIN + ratio * (PWM_MAX - PWM_MIN))
    state["duty"] = duty

    state["uptime"] = int(time.time() - start_time)
    state["hours"] = state["uptime"] / 3600.0

# ===== UDP SIMHUB =====
udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

def send_udp():
    pkt = f"$SH;temp={state['temp']:.2f};duty={state['duty']};state={state['state']};relay={int(state['relay'])};uptime={state['uptime']}\n"
    try:
        udp_sock.sendto(pkt.encode(), (UDP_HOST, UDP_PORT))
    except Exception as e:
        print(f"[UDP] erro: {e}")

# ===== HTTP / WEBSOCKET =====
async def index(request):
    # Carrega dashboard.h e extrai o HTML embutido
    dash = Path(__file__).parent.parent / "include" / "dashboard.h"
    if dash.exists():
        content = dash.read_text(encoding="utf-8")
        # Extrai o conteúdo entre R"rawliteral( ... )rawliteral"
        start = content.find('R"rawliteral(')
        end = content.rfind(')rawliteral"')
        if start != -1 and end != -1:
            html = content[start + len('R"rawliteral('):end]
            return web.Response(text=html, content_type="text/html")
    return web.Response(text="<h1>dashboard.h não encontrado</h1>", content_type="text/html")

async def api_telemetry(request):
    return web.json_response(state)

async def ws_handler(request):
    ws = web.WebSocketResponse()
    await ws.prepare(request)
    ws_clients.add(ws)
    print(f"[WS] cliente conectado ({len(ws_clients)} ativos)")
    try:
        async for msg in ws:
            if msg.type == WSMsgType.ERROR:
                break
    finally:
        ws_clients.discard(ws)
        print(f"[WS] cliente desconectado ({len(ws_clients)} ativos)")
    return ws

async def broadcast_loop():
    while True:
        if ws_clients:
            payload = json.dumps(state)
            dead = []
            for ws in ws_clients:
                try:
                    await ws.send_str(payload)
                except Exception:
                    dead.append(ws)
            for ws in dead:
                ws_clients.discard(ws)
        await asyncio.sleep(WS_INTERVAL_MS / 1000)

async def physics_loop():
    while True:
        simulate_physics()
        update_control()
        await asyncio.sleep(TICK_MS / 1000)

async def udp_loop():
    while True:
        send_udp()
        await asyncio.sleep(UDP_INTERVAL_MS / 1000)

async def main():
    app = web.Application()
    app.router.add_get("/", index)
    app.router.add_get("/api/telemetry", api_telemetry)
    app.router.add_get("/ws", ws_handler)

    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, "0.0.0.0", HTTP_PORT)
    await site.start()

    print(f"🌡️  Simulador rodando em http://localhost:{HTTP_PORT}")
    print(f"📡 UDP broadcast em {UDP_HOST}:{UDP_PORT}")
    print(f"🔌 WebSocket em ws://localhost:{HTTP_PORT}/ws")
    print("Pressione Ctrl+C para parar.\n")

    await asyncio.gather(physics_loop(), broadcast_loop(), udp_loop())

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n👋 Simulador encerrado.")
