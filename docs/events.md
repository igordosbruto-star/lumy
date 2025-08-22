# Eventos — Lumy (M0)

Formato lógico:
- Evento = { pages: [ Page ] }
- Page = { conditions, trigger, commands[] }
- conditions: { switch?: {id, value}, var?: {id, op, value}, time?: {hourMin,hourMax}, region?: int }
- trigger: "action" | "touch" | "autorun" | "parallel"
- command = { cmd: string, ...args }

Comandos (10 básicos):
1. ShowText { text: string, face?: string, speed?: int }
2. SetSwitch { id: int, value: bool }
3. SetVar   { id: int, op: "set"|"add"|"sub"|"mul"|"div", value: int }
4. If       { cond: {switch?|var?}, then: [command], else?: [command] }
5. Wait     { ms: int }
6. TransferPlayer { map: string, x: int, y: int, fade?: "none"|"black"|"white" }
7. PlayBGM  { name: string, volume?: int, loop?: bool }
8. PlaySE   { name: string, volume?: int }
9. ShowPicture { id:int, file:string, x:int, y:int, scale?:int, opacity?:int, layer?:"belowUI"|"UI" }
10. ErasePicture { id:int }

Variáveis:
- Globais do projeto; Locais do evento (prefixo "local:" para diferenciar, ex.: "local:1").

Exemplo de Page:
{
  "conditions": { "switch": {"id":1,"value":true} },
  "trigger": "action",
  "commands": [
    {"cmd":"ShowText","text":"Bem-vindo à Vila de Nascen!"},
    {"cmd":"SetSwitch","id":2,"value":true},
    {"cmd":"If","cond":{"switch":{"id":2,"value":true}},"then":[
      {"cmd":"ShowText","text":"O portão foi aberto."},
      {"cmd":"TransferPlayer","map":"forest.tmx","x":10,"y":5}
    ]}
  ]
}