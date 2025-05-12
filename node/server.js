const net = require("net");
const WebSocket = require("ws");
const express = require("express");
const http = require("http");

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });
let wsClients = [];

wss.on("connection", (ws) => {
  wsClients.push(ws);
  ws.on("close", () => {
    wsClients = wsClients.filter((client) => client !== ws);
  });
});

let buffer = Buffer.alloc(0);

const client = net.createConnection({ port: 8000, host: "127.0.0.1" }, () => {
  console.log("[INFO] Connected to C++ server");
});

client.on("data", (chunk) => {
  buffer = Buffer.concat([buffer, chunk]);

  while (buffer.length >= 4) {
    const size = buffer.readInt32LE(0);
    if (buffer.length >= size + 4) {
      const imageBuffer = buffer.slice(4, 4 + size);
      buffer = buffer.slice(4 + size);

      const base64 = imageBuffer.toString("base64");
      wsClients.forEach((ws) => {
        if (ws.readyState === WebSocket.OPEN) {
          ws.send(base64);
        }
      });
    } else break;
  }
});

client.on("end", () => {
  console.log("[INFO] C++ server disconnected");
});

client.on("error", (err) => {
  console.log("[ERROR] " + err.message);
});

app.use(express.static(__dirname + "/public"));
server.listen(3000, () => {
  console.log("[INFO] Web server: http://localhost:3000");
});
