// ════════════════════════════════════════════════════════════════════════════
//  RS50 Thermal Controller — dashboard.h
//  HTML + CSS + JS do dashboard web (separado do .ino para evitar
//  problemas de parsing do raw literal pelo arduino-cli)
// ════════════════════════════════════════════════════════════════════════════

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <Arduino.h>

const char HTML_PAGE[] PROGMEM = R"RS50DASH338(
<!DOCTYPE html>
<html lang="pt-BR"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>RS50 Thermal Monitor</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.1/dist/chart.umd.min.js"></script>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Segoe UI',system-ui,sans-serif;background:#0a0a0f;color:#e0e0e0;min-height:100vh;padding:20px}
.container{max-width:1100px;margin:0 auto}
h1{font-size:1.5em;margin-bottom:5px;color:#4af}
.subtitle{font-size:0.85em;color:#888;margin-bottom:20px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:15px;margin-bottom:20px}
.card{background:#15151f;border-radius:12px;padding:18px;border:1px solid #2a2a3a;transition:all 0.3s}
.card:hover{border-color:#4af}
.card-label{font-size:0.8em;color:#888;text-transform:uppercase;letter-spacing:0.5px}
.card-value{font-size:2em;font-weight:700;margin-top:5px;line-height:1}
.card-unit{font-size:0.5em;color:#888;margin-left:5px}
.state{font-size:1.3em;font-weight:600;padding:8px 16px;border-radius:8px;display:inline-block;margin-top:8px}
.NORMAL{background:#1a4a1a;color:#6f6}
.WARMING{background:#1a3a4a;color:#6cf}
.WARNING{background:#4a3a1a;color:#fc6}
.CRITICAL{background:#4a1a1a;color:#f66;animation:blink 0.5s infinite}
.SHUTDOWN{background:#6a0000;color:#fff;animation:blink 0.2s infinite}
.FAULT{background:#4a1a4a;color:#f6f;animation:blink 0.4s infinite}
@keyframes blink{50%{opacity:0.4}}
.bar{width:100%;height:20px;background:#222;border-radius:10px;overflow:hidden;margin-top:8px}
.bar-fill{height:100%;transition:width 0.5s,background 0.3s;background:linear-gradient(90deg,#4af 0%,#4f4 30%,#fc4 60%,#f44 100%)}
.chart-card{background:#15151f;border-radius:12px;padding:20px;border:1px solid #2a2a3a;margin-bottom:20px}
.chart-card h2{font-size:1em;color:#888;margin-bottom:12px;text-transform:uppercase;letter-spacing:0.5px}
.chart-wrap{position:relative;height:280px}
.footer{text-align:center;color:#555;font-size:0.8em;margin-top:30px}
.connection{position:fixed;top:10px;right:10px;padding:5px 10px;border-radius:5px;font-size:0.8em;z-index:10}
.online{background:#1a4a1a;color:#6f6}
.offline{background:#4a1a1a;color:#f66}
</style></head><body>
<div class="container">
<h1>RS50 Thermal Monitor</h1>
<div class="subtitle">DD Wheel + ODESC FFBeast | Firmware <span id="fw">--</span></div>
<div id="conn" class="connection offline">Conectando...</div>
<div class="grid">
<div class="card"><div class="card-label">Estado</div><div id="state" class="state NORMAL">--</div></div>
<div class="card"><div class="card-label">Eixo</div><div class="card-value"><span id="tShaft">--</span><span class="card-unit">C</span></div></div>
<div class="card"><div class="card-label">Estator Estimado</div><div class="card-value"><span id="tStator">--</span><span class="card-unit">C</span></div><div class="bar"><div id="barStator" class="bar-fill" style="width:0%"></div></div></div>
<div class="card"><div class="card-label">dT/dt</div><div class="card-value"><span id="dtdt">--</span><span class="card-unit">C/min</span></div></div>
<div class="card"><div class="card-label">Fan PWM</div><div class="card-value"><span id="pwm">--</span><span class="card-unit">%</span></div><div class="bar"><div id="barPwm" class="bar-fill" style="width:0%"></div></div></div>
<div class="card"><div class="card-label">Pico</div><div class="card-value"><span id="peak">--</span><span class="card-unit">C</span></div></div>
<div class="card"><div class="card-label">Motor</div><div class="card-value" id="motor">--</div></div>
<div class="card"><div class="card-label">Horas Totais</div><div class="card-value"><span id="hours">--</span><span class="card-unit">h</span></div></div>
</div>
<div class="chart-card">
<h2>Historico (ultimos 5 min)</h2>
<div class="chart-wrap"><canvas id="chart"></canvas></div>
</div>
<div class="footer">RS50 Thermal Controller</div>
</div>
<script>
const MAX_POINTS = 300;
const ctx = document.getElementById('chart').getContext('2d');
const chart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [
      { label: 'Eixo (C)',    data: [], borderColor:'#4af', backgroundColor:'rgba(68,170,255,0.1)', tension:0.3, pointRadius:0, borderWidth:2 },
      { label: 'Estator (C)', data: [], borderColor:'#f66', backgroundColor:'rgba(255,102,102,0.1)', tension:0.3, pointRadius:0, borderWidth:2 },
      { label: 'Fan (%)',     data: [], borderColor:'#6f6', backgroundColor:'rgba(102,255,102,0.1)', tension:0.3, pointRadius:0, borderWidth:2, yAxisID:'y2' }
    ]
  },
  options: {
    responsive: true, maintainAspectRatio: false, animation: false,
    interaction: { intersect: false, mode: 'index' },
    plugins: {
      legend: { labels: { color:'#aaa' } },
      tooltip: { backgroundColor:'#15151f', borderColor:'#4af', borderWidth:1 }
    },
    scales: {
      x: { ticks:{color:'#666',maxTicksLimit:6}, grid:{color:'#222'} },
      y: { position:'left', title:{display:true,text:'Temperatura',color:'#888'}, ticks:{color:'#666'}, grid:{color:'#222'} },
      y2:{ position:'right', title:{display:true,text:'Fan %',color:'#888'}, min:0, max:100, ticks:{color:'#666'}, grid:{display:false} }
    }
  }
});

let ws;
function connect(){
  ws = new WebSocket('ws://' + location.hostname + ':81');
  ws.onopen = function() {
    document.getElementById('conn').className='connection online';
    document.getElementById('conn').textContent='Online';
  };
  ws.onclose = function() {
    document.getElementById('conn').className='connection offline';
    document.getElementById('conn').textContent='Offline (reconectando)';
    setTimeout(connect, 2000);
  };
  ws.onmessage = function(e) {
    const d = JSON.parse(e.data);
    document.getElementById('fw').textContent = d.fw;
    document.getElementById('state').textContent = d.state;
    document.getElementById('state').className = 'state ' + d.state;
    document.getElementById('tShaft').textContent = d.tShaft.toFixed(1);
    document.getElementById('tStator').textContent = d.tStator.toFixed(1);
    document.getElementById('dtdt').textContent = (d.dtdt*60).toFixed(2);
    document.getElementById('pwm').textContent = d.pwm;
    document.getElementById('peak').textContent = d.peak.toFixed(1);
    document.getElementById('motor').textContent = d.motor ? 'ON' : 'OFF';
    document.getElementById('hours').textContent = d.hours;
    document.getElementById('barStator').style.width = Math.min(100,(d.tStator/70)*100)+'%';
    document.getElementById('barPwm').style.width = d.pwm+'%';
    const t = new Date().toLocaleTimeString('pt-BR',{hour:'2-digit',minute:'2-digit',second:'2-digit'});
    chart.data.labels.push(t);
    chart.data.datasets[0].data.push(d.tShaft);
    chart.data.datasets[1].data.push(d.tStator);
    chart.data.datasets[2].data.push(d.pwm);
    if (chart.data.labels.length > MAX_POINTS) {
      chart.data.labels.shift();
      chart.data.datasets.forEach(function(ds) { ds.data.shift(); });
    }
    chart.update('none');
  };
}
connect();
</script></body></html>
)RS50DASH338";

#endif // DASHBOARD_H
