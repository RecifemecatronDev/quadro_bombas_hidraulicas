#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Interface de controle</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
        }

        header {
            background-color: #f3a66e;
            padding: 10px;
            text-align: center;
            color: #FFF;
        }

        footer {
            background-color: #f3a66e;
            padding: 10px;
            text-align: center;
            color: #FFF;
        }

        #relayControls {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            grid-gap: 10px;
            width: 80%;
            margin: auto;
        }

        .button {
            background-color: #f3a66e;
            color: #FFF;
            border: none;
            padding: 10px 20px;
            text-align: center;
            text-decoration: none;
            display: block;
            font-size: 16px;
            margin-bottom: 10px;
            border-radius: 5px;
            cursor: pointer;
        }

        .button-final {
            background-color: #f3a66e;
            color: #FFF;
            border: none;
            padding: 10px 20px;
            text-align: center;
            text-decoration: none;
            display: block;
            font-size: 16px;
            border-radius: 5px;
            cursor: pointer;
            width: 80%;
            margin: 0.5rem auto;
        }


        .button:hover {
            background-color: #f68640;
            color: #FFF;
            transition: 1s;
        }

        .button-final:hover {
            background-color: #f68640;
            color: #FFF;
            transition: 1s;
        }

        #additionalButtons {
            margin-bottom: 5rem;
            margin-top: 5rem;
        }


        #configurations {
            display: flex;
            flex-wrap: wrap;
            justify-content: space-between;
            margin: 1rem auto;
            width: 80%;
        }

        #configurations h2 {
            width: 100%;
            text-align: center;
        }

        #configurations form {
            display: block;
            align-items: center;
            justify-content: space-between;
            flex-basis: 45%;
            padding: 1rem;
            border: 1px solid #f3a66e;
            border-radius: 5px;
        }

        #configurations form select,
        #configurations form input {
            width: 45%;
        }

        @media screen and (max-width: 768px) {
            #configurations form {
                flex-basis: 100%;
                margin-bottom: 1rem;
            }
        }
    </style>
</head>
<body>
<header>
    <h2 style="margin: 0;">Sistema de automatização de bombas</h2>
    <p>RecifeMecatron</p>
</header>
<p id="lastMessage"></p>

<!--
<div id="relayControls">
    <button class="button" onclick="sendCommand('/relay?index=0&state=1')">Ligar relé 0</button>
    <button class="button" onclick="sendCommand('/relay?index=0&state=0')">Desligar relé 0</button>
    <button class="button" onclick="sendCommand('/relay?index=1&state=1')">Ligar relé 1</button>
    <button class="button" onclick="sendCommand('/relay?index=1&state=0')">Desligar relé 1</button>
    <button class="button" onclick="sendCommand('/relay?index=2&state=1')">Ligar relé 2</button>
    <button class="button" onclick="sendCommand('/relay?index=2&state=0')">Desligar relé 2</button>
</div>
-->
<div id="additionalButtons">
    <button class="button-final" onclick="sendCommand('/command?command=STATUS')">Verificar status dos relés</button>
    <button class="button-final" onclick="sendCommand('/command?command=DATAHORA')">Imprimir data e hora atual</button>
    <!--
    <button class="button-final" onclick="sendCommand('/command?command=GETIRRIGATIONTIMES')">Obter horários de ativação do relé</button>
    <button class="button-final" onclick="sendCommand('/command?command=SAVETIMES')">Salvar horários de ativação do relé</button>
    <button class="button-final" onclick="sendCommand('/command?command=LOADTIMES')">Carregar horários de ativação do relé</button>
    <button class="button-final" onclick="sendCommand('/command?command=TEMPERATURE')">Obter a temperatura do sistema</button>
    -->
</div>


<div id="configurations">
    <h2>Linha de comando</h2>
<form id="command_line">
<div>
<input type="text" name="command" style="width:100%">
</div>
<div>
<input type="submit" value="enviar comando">
</div>
</form>

    <h2>Ajustar data e hora do dispositivo</h2>
    <form id="formAdjustDateTime">
        <label>Insita a hora atualizada</label>
        <input type="hidden" name="type" value="AJUSTARDATAHORA">
        <input type="text" id="dateTimeInput" placeholder="DD/MM/YYYY HH:MM:SS" name="dateTime" required>
        <button type="submit" class="button-final">Enviar</button>
    </form>

    <h2>Ajustar credenciais do Wifi</h2>
    <button id='scanNetworks' onclick='fetchNetworks()'>Buscar redes</button>
    <ul id='networksList'></ul>
    <form id='wifiCredentials' action="/setWiFi" method="post">
        <label for='ssid'>SSID:</label><br>
        <input id='ssid' type="text" name="ssid">
        <br>
        <label for='password'>Password:</label><br>
        <input type="password" name="password">
        <br><br>
        <input type="submit" value="Submit">
    </form>
</div>

<footer>
    <p>www.recifemecatron.com</p>
    <p>@rmecatron</p>
    <p>Suporte técnico: (81)98797.6280 / (81)99646.0691</p>
</footer>

<script>
    function sendCommand(command) {
        fetch(command)
            .then(response => response.text())
            .then(message => {
                const messageElement = document.getElementById('lastMessage');
                messageElement.textContent = message;
            });
    }

    function fetchLastMessage() {
        fetch('/lastMessage')
            .then(response => response.text())
            .then(message => {
                const messageElement = document.getElementById('lastMessage');
                messageElement.textContent = message;
            });
    }

    setInterval(fetchLastMessage, 1000);  // Verifica a cada 1 segundo

    document.getElementById("command_line").addEventListener("submit", function (event){
        event.preventDefault();

        let form = new FormData(this);

        sendCommand(`/command?command=${form.get("command")}`)
    })

    document.getElementById("formTime").addEventListener("submit", function (event){
        event.preventDefault();

        let form = new FormData(this);

        sendCommand(`/command?command=${form.get("type")},${form.get("relay")},${form.get("time")}`)
    })

    document.getElementById("formMilliseconds").addEventListener("submit", function (event){
        event.preventDefault();

        let form = new FormData(this);

        sendCommand(`/command?command=${form.get("type")},${form.get("time")}`)
    })

    document.getElementById("formAdjustDateTime").addEventListener("submit", function (event){
        event.preventDefault();

        let form = new FormData(this);

        sendCommand(`/command?command=${form.get("type")},${form.get("dateTime")}`)
    })

    document.getElementById('timeInput').addEventListener('input', function(e) {
        let x = e.target.value.replace(/\D/g, '').match(/(\d{0,2})(\d{0,2})/);
        e.target.value = '';
        if (x[1] !== '') {
            e.target.value += x[1] + (x[2] !== '' ? ':' : '');
        }
        if (x[2] !== '') {
            e.target.value += x[2];
        }
    });

    document.getElementById('millisecondsInput').addEventListener('keypress', function(e) {
        if (e.key < '0' || e.key > '9') {
            e.preventDefault();
        }
    });

</script>
<script>
    function fetchNetworks() {
      document.getElementById('scanNetworks').textContent = 'Buscando...';
      fetch('/scanWiFi')
        .then(response => response.json())
        .then(data => {
          var networksList = document.getElementById('networksList');
          networksList.innerHTML = '';
          data.forEach(network => {
            var listItem = document.createElement('li');
            var networkButton = document.createElement('button');
            networkButton.textContent = network.ssid + ' (' + network.rssi + ' dBm)';
            networkButton.onclick = function() { selectNetwork(network.ssid); };
            listItem.appendChild(networkButton);
            networksList.appendChild(listItem);
          });
          document.getElementById('scanNetworks').textContent = 'Buscar redes';
        });
    }

    function selectNetwork(ssid) {
        document.getElementById('ssid').value = ssid;
    }
</script>
</body>
</html>)rawliteral";