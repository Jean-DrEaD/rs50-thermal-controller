#pragma once
#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"H(<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>RS50 Thermal Dashboard</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body{font-family:system-ui,sans-serif;margin:0;background:#0f172a;color:#e2e8f0}
  header{background:#1e293b;padding:1rem;text-align:center}
  main{padding:1rem;max-width:800px;margin:0 auto}
  .card{background:#1e293b;border-radius:8px;padding:1rem;margin-bottom:1rem}
  .temp{font-size:2.5rem;font-weight:bold;color:#38bdf8}
  footer{text-align:center;padding:1rem;color:#64748b;font-size:.85rem}
</style>
</head>
<body>
<header><h1>RS50 Thermal Controller</h1></header>
<main>
  <div class="card">
    <div>Temperatura atual</div>
    <div class="temp" id="temp">--.- °C</div>
  </div>
  <div class="card">
    <div>Setpoint: <span id="sp">--</span> °C</div>
    <div>Estado: <span id="state">--</span></div>
  </div>
</main>
<footer>RS50 &middot; Firmware <span id="fw">--</span></footer>
<script>
  function fetchStatus() {
    fetch('/api/status')
      .then(function(r){ return r.json(); })
      .then(function(d){
        document.getElementById('temp').textContent  = d.temp.toFixed(1) + ' °C';
        document.getElementById('sp').textContent    = d.setpoint;
        document.getElementById('state').textContent = d.state;
        if (d.version) document.getElementById('fw').textContent = d.version;
      })
      .catch(function(e){ console.error('status err', e); });
  }
  setInterval(fetchStatus, 2000);
  fetchStatus();
</script>
</body>
</html>)H";