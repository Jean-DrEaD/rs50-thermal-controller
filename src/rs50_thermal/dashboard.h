#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>RS50 Thermal Dashboard</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: 'Segoe UI', Roboto, sans-serif;
    background: linear-gradient(135deg, #0f2027, #203a43, #2c5364);
    color: #e0e0e0;
    min-height: 100vh;
    padding: 20px;
  }
  .container { max-width: 900px; margin: 0 auto; }
  h1 {
    text-align: center;
    margin-bottom: 30px;
    font-size: 2em;
    background: linear-gradient(90deg, #00c6ff, #0072ff);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
  }
  .grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 20px;
    margin-bottom: 30px;
  }
  .card {
    background: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 12px;
    padding: 20px;
    backdrop-filter: blur(10px);
    transition: transform 0.2s;
  }
  .card:hover { transform: translateY(-3px); }
  .label {
    font-size: 0.85em;
    text-transform: uppercase;
    color: #8ab4f8;
    margin-bottom: 8px;
    letter-spacing: 1px;
  }
  .value {
    font-size: 2.2em;
    font-weight: bold;
    color: #fff;
  }
  .unit { font-size: 0.5em; color: #aaa; }
  .state-IDLE     { color: #4caf50; }
  .state-WARMING  { color: #ffc107; }
  .state-WARNING  { color: #ff9800; }
  .state-CRITICAL { color: #f44336; animation: blink 1s infinite; }
  @keyframes blink { 50% { opacity: 0.4; } }
  .relay-on  { color: #f44336; }
  .relay-off { color: #4caf50; }
  .bar {
    width: 100%;
    height: 10px;
    background: rgba(255,255,255,0.1);
    border-radius: 5px;
    overflow: hidden;
    margin-top: 10px;
  }
  .bar-fill {
    height: 100%;
    background: linear-gradient(90deg, #00c6ff, #0072ff);
    transition: width 0.4s ease;
  }
  .status {
    text-align: center;
    padding: 10px;
    border-radius: 8px;
    margin-top: 20px;
    font-size: 0.9em;
  }
  .connected    { background: rgba(76,175,80,0.2); color: #4caf50; }
  .disconnected { background: rgba(244,67,54,0.2); color: #f44336; }
  canvas {
    width: 100%;
    height: 200px;
    background: rgba(0,0,0,0.3);
    border-radius: 12px;
    margin-top: 20px;
  }
</style>
</head>
<body>
<div class="container">
  <h1>🌡️ RS50 Thermal Controller</h1>

  <div class="grid">
    <div class="card">
      <div class="label">Temperatura</div>
      <div class="value"><span id="temp">--</span><span class="unit">°C</span></div>
      <div class="bar"><div class="bar-fill" id="tempBar" style="width:0%"></div></div>
    </div>
    <div class="card">
      <div class="label">PWM Duty</div>
      <div class="value"><span id="duty">--</span><span class="unit">%</span></div>
      <div class="bar"><div class="bar-fill" id="dutyBar" style="width:0%"></div></div>
    </div>
    <div class="card">
      <div class="label">Estado Térmico</div>
      <div class="value" id="state">--</div>
    </div>
    <div class="card">
      <div class="label">Relé Fail-Safe</div>
      <div class="value" id="relay">--</div>
    </div>
    <div class="card">
      <div class="label">Uptime</div>
      <div class="value" id="uptime" style="font-size:1.4em">--</div>
    </div>
  </div>

  <canvas id="chart"></canvas>
  <div id="status" class="status disconnected">🔌 Desconectado</div>
</div>

<script>
  const tempEl   = document.getElementById('temp');
  const dutyEl   = document.getElementById('duty');
  const stateEl  = document.getElementById('state');
  const relayEl  = document.getElementById('relay');
  const uptimeEl = document.getElementById('uptime');
  const tempBar  = document.getElementById('tempBar');
  const dutyBar  = document.getElementById('dutyBar');
  const statusEl = document.getElementById('status');
  const canvas   = document.getElementById('chart');
  const ctx      = canvas.getContext('2d');

  const history = [];
  const MAX_PTS = 120;

  function fmtUptime(s) {
    const h = Math.floor(s/3600);
    const m = Math.floor((s%3600)/60);
    const sec = s%60;
    return `${h}h ${m}m ${sec}s`;
  }

  function drawChart() {
    const w = canvas.width = canvas.offsetWidth;
    const h = canvas.height = canvas.offsetHeight;
    ctx.clearRect(0,0,w,h);
    if (history.length < 2) return;

    // Grade
    ctx.strokeStyle = 'rgba(255,255,255,0.1)';
    ctx.lineWidth = 1;
    for (let i = 0; i <= 4; i++) {
      const y = (h/4)*i;
      ctx.beginPath();
      ctx.moveTo(0, y); ctx.lineTo(w, y);
      ctx.stroke();
    }

    // Linha de temperatura
    ctx.strokeStyle = '#00c6ff';
    ctx.lineWidth = 2;
    ctx.beginPath();
    history.forEach((v, i) => {
      const x = (i/(MAX_PTS-1)) * w;
      const y = h - ((v - 20) / 80) * h; // escala 20-100°C
      i ? ctx.lineTo(x,y) : ctx.moveTo(x,y);
    });
    ctx.stroke();
  }

  function connect() {
    const ws = new WebSocket(`ws://${location.hostname}:81/`);

    ws.onopen = () => {
      statusEl.textContent = '✅ Conectado';
      statusEl.className = 'status connected';
    };
    ws.onclose = () => {
      statusEl.textContent = '🔌 Desconectado — tentando reconectar...';
      statusEl.className = 'status disconnected';
      setTimeout(connect, 2000);
    };
    ws.onmessage = (ev) => {
      try {
        const d = JSON.parse(ev.data);
        tempEl.textContent   = d.temp.toFixed(2);
        dutyEl.textContent   = d.duty;
        stateEl.textContent  = d.state;
        stateEl.className    = 'value state-' + d.state;
        relayEl.textContent  = d.relay ? 'ATIVADO' : 'NORMAL';
        relayEl.className    = 'value ' + (d.relay ? 'relay-on' : 'relay-off');
        uptimeEl.textContent = fmtUptime(d.uptime);
        tempBar.style.width  = Math.min(100, (d.temp/100)*100) + '%';
        dutyBar.style.width  = d.duty + '%';

        history.push(d.temp);
        if (history.length > MAX_PTS) history.shift();
        drawChart();
      } catch(e) { console.error(e); }
    };
  }

  window.addEventListener('resize', drawChart);
  connect();
</script>
</body>
</html>
)rawliteral";

#endif // DASHBOARD_H
