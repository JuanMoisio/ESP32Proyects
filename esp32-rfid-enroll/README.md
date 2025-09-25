# ESP32 RFID Enroll

Firmware para **ESP32 + RC522** que permite enrolar, gestionar y validar tarjetas RFID.  
Incluye un **servidor web embebido** con interfaz básica y una **API REST** para integrarse con aplicaciones externas (.NET, web, mobile, etc.).

---

## 📡 Conexión

- **Protocolo:** HTTP (LAN)
- **Puerto:** 80
- **Base URL:** `http://<IP_DEL_ESP32>` (ej: `http://192.168.0.23`)  
  > Si tenés mDNS habilitado: `http://esp32.local`
- **Formato:** JSON
- **Auth mínima:** Header `X-PIN: <pin>` en operaciones sensibles (enrolar/borrar).

---

## 🔑 API REST

### Estado del dispositivo
**GET** `/api/status`  
```json
{
  "ip": "192.168.0.23",
  "armed": { "enroll": true, "delete": false },
  "cardsCount": 12
}


Listar tarjetas
GET /api/cards
[
  { "uid": "DEADBEEF", "name": "Juan" },
  { "uid": "33364BAC", "name": "Visita" }
]



Armar ENROLL (próxima tarjeta se registra)
POST /api/arm-enroll
Headers: X-PIN: 1234
Body:
{ "name": "Juan" }
Respuesta:
{ "ok": true, "msg": "ENROLL armed" }

Armar DELETE (próxima tarjeta se da de baja)
POST /api/arm-delete
Headers: X-PIN: 1234
Body: (vacío)
{ "ok": true, "msg": "DELETE armed" }
Borrar tarjeta por UID
POST /api/delete
Headers: X-PIN: 1234
Body:
{ "uid": "DEADBEEF" }
Respuesta OK:
{ "ok": true }
Respuesta error:
{ "ok": false, "error": "not_found" }
Chequear acceso de un UID
POST /api/check
Body:
{ "uid": "DEADBEEF" }
Respuesta:
{ "uid": "DEADBEEF", "access": true, "name": "Juan" }
o
{ "uid": "33364BAC", "access": false }
Forzar reporte de próxima tarjeta (modo test)
POST /api/send
Body: (vacío)
{ "ok": true }
⚠️ Códigos de error
200 OK → operación exitosa.
400 Bad Request → JSON inválido o campos faltantes.
403 Forbidden → PIN inválido o ausente.
404 Not Found → UID inexistente.
500 Internal Server Error → error inesperado.
📱 Ejemplos de uso
cURL
# Estado
curl http://esp32.local/api/status

# Listar
curl http://esp32.local/api/cards

# Enrolar próxima tarjeta
curl -X POST http://esp32.local/api/arm-enroll \
  -H "Content-Type: application/json" -H "X-PIN: 1234" \
  -d '{"name":"Juan"}'

# Dar de baja próxima tarjeta
curl -X POST http://esp32.local/api/arm-delete -H "X-PIN: 1234"

# Borrar tarjeta por UID
curl -X POST http://esp32.local/api/delete \
  -H "Content-Type: application/json" -H "X-PIN: 1234" \
  -d '{"uid":"DEADBEEF"}'

# Chequear acceso
curl -X POST http://esp32.local/api/check \
  -H "Content-Type: application/json" \
  -d '{"uid":"33364BAC"}'
Cliente .NET (HttpClient)
Ejemplo con HttpClient en .NET 6+:
using System.Net.Http.Json;

// Inicializar cliente
var http = new HttpClient { BaseAddress = new Uri("http://esp32.local/") };

// Estado
var status = await http.GetFromJsonAsync<EspStatus>("api/status");
Console.WriteLine($"IP={status.Ip} Cards={status.CardsCount}");

// Enrolar próxima tarjeta
var req = new HttpRequestMessage(HttpMethod.Post, "api/arm-enroll") {
    Content = JsonContent.Create(new { name = "Juan" })
};
req.Headers.Add("X-PIN", "1234");
var res = await http.SendAsync(req);
Console.WriteLine(await res.Content.ReadAsStringAsync());
DTO sugeridos:
public sealed class EspStatus {
    public string Ip { get; set; } = "";
    public ArmedState Armed { get; set; } = new();
    public int CardsCount { get; set; }
    public sealed class ArmedState {
        public bool Enroll { get; set; }
        public bool Delete { get; set; }
    }
}

public sealed class CardDto {
    public string Uid { get; set; } = "";
    public string Name { get; set; } = "";
}
🛠️ Recomendaciones
Siempre enviar UID en HEX mayúscula (ej: "DEADBEEF").
Configurar timeout corto (2–5s) en la app cliente.
Usar X-PIN solo en redes seguras (LAN).
Tras usar arm-enroll o arm-delete, la acción se consume con la próxima tarjeta y se desarma automáticamente.
🚀 Futuro
WebSocket API para eventos en tiempo real ({"type":"card","uid":"..."}).
Export/Import CSV de tarjetas.
Autenticación fuerte (token o Basic Auth) para accesos remotos.